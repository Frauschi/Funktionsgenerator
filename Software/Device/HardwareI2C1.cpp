
#include <HardwareI2C1.h>
#include <cstdint>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareCore.h"


//---------------------------------------------------------------------------------------
// -------------------- Implementation of Class 'HardwareI2C1' --------------------------

Device::HardwareI2C1::
HardwareI2C1()
{
	using namespace Device;
	/* 	I2C1:
	 * 		GPIO	PB8			I2C1_SCL
	 * 				PB9			I2C1_SDA
	 *
	 *		DMA 1 	Channel 6	I2C1_Tx
	 *				Channel 7	I2C1_Rx
	 */

	/* Enable clocks for GPIOB, DMA1 and I2C1 */
	RCC->AHB2ENR	|= 0x01<<1;		/* GPIOB enable */
	RCC->AHB1ENR	|= 0x01<<0; 	/* DMA1 enable */
	RCC->APB1ENR1	|= 0x01<<21; 	/* I2C1 enable */

	/* Configure GPIOB Pin 8 (SCL) and 9 (SDA) */
	GPIOB->AFR[1]	 =  (GPIOB->AFR[1] & ~(0xFF)) | (0x04<<0 | 0x04<<4);
	GPIOB->MODER	 = (GPIOB->MODER & ~(0x0F<<16)) | (0x02<<16 | 0x02<<18);
	GPIOB->OTYPER	|= (0x01<<8 | 0x01<<9);
	GPIOB->OSPEEDR	|= (0x03<<16 | 0x03<<18);
	GPIOB->PUPDR	&= ~(0x03<<16 | 0x03<<18);	/* Disable internal pull ups */

	/* Configure DMA1 channel 6 (Tx) */
	/* Medium prio, 8 bit memory / peripheral size, memory increment mode, read from memory, Channel disabled
	 */
	DMA1_Channel6->CCR	|= (0x01<<12 | 0x01<<7 | 0x01<<4);
	DMA1_Channel6->CPAR	 = reinterpret_cast<std::uint32_t>(&I2C1->TXDR);	/* I2C1 Tx data register is destination address */

	/* Configure DMA1 channel 7 (Rx) */
	/* Medium prio, 8 bit memory / peripheral size, memory increment mode, read from peripheral, Channel disabled
	 */
	DMA1_Channel7->CCR	|= (0x01<<12 | 0x01<<7);
	DMA1_Channel7->CPAR	 = reinterpret_cast<std::uint32_t>(&I2C1->RXDR);	/* I2C1 Rx data register is source address */

	DMA1_CSELR->CSELR	|= (0x03<<20 | 0x03<<24);	/* DMA1 Channel 6 mapped to I2C1_Tx, Channel 7 mapped to I2C1_Rx */

	/* Disable I2C1 */
	I2C1->CR1		&= ~(0x01<<22 | 0x01<<0);

	/* Enable Fast mode plus driving capabilities */
	SYSCFG->CFGR1	|= (0x01<<20 | 0x01<<19 | 0x01<<18);

	/* Set proper timing: Fast-mode (400kHz) */
	enum { PRESC 	= 1 };
	enum { SCLL  	= 9 };
	enum { SCLH  	= 3 };
	enum { SDADEL  	= 1 };
	enum { SCLDEL  	= 3 };

	I2C1->TIMINGR	 = (PRESC<<28 | SCLDEL<<20 | SDADEL<<16 | SCLH<<8 | SCLL<<0);

	/* Enable Own Address register 1 (done in HAL, too) */
	I2C1->OAR1		|= 0x01<<15;

	/* Enable AUTOEND */
	I2C1->CR2		|= 0x01<<25;

	/* NBYTES to send*/
	I2C1->CR2		|= 0x01<<16;

	/* Enable I2C1 */
	I2C1->CR1		|= 0x01<<0;

	/* Configure I2C1 interrupts */
	InterruptMgr::reference().addHandlerForInterrupt(InterruptId::I2C1_EVInt, [this]() { this->i2cEventHandler(); });
	InterruptMgr::reference().addHandlerForInterrupt(InterruptId::I2C1_ERInt, [this]() { this->i2cErrorHandler(); });

	/* Configure DMA interrupts */
	InterruptMgr::reference().addHandlerForInterrupt(InterruptId::DMA1_CH6Int, [this]() { this->dmaTxHandler(); });
	InterruptMgr::reference().addHandlerForInterrupt(InterruptId::DMA1_CH7Int, [this]() { this->dmaRxHandler(); });
 }


Device::HardwareI2C1::
~HardwareI2C1()
{
	// Disable interrupts
	Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::I2C1_EVInt);
	Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::I2C1_ERInt);
	Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::DMA1_CH6Int);
	Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::DMA1_CH7Int);

	// Disable clock
	RCC->APB1ENR1	&= ~(0x01<<21);
}


void Device::HardwareI2C1::
beginTransmit(const std::uint8_t slaveAddress, const std::uint8_t* dataSrc, const std::size_t numOfBytes) const
{
	/* Configure DMA Tx Channel and enable it */
	DMA1->IFCR			|= 0x01<<20;
	DMA1_Channel6->CNDTR = numOfBytes;
	DMA1_Channel6->CMAR	 = reinterpret_cast<std::uint32_t>(dataSrc);
	DMA1_Channel6->CCR	|= (0x01<<3 | 0x01<<1 | 0x01<<0); /* Transfer error and Transfer complete interrupt enable */


	/* Configure Interrupts and DMA usage */
	I2C1->CR1	|= (0x01<<14 | 0x01<<7 | 0x01<<6 | 0x01<<4);

	/* Configure Transmission parameters and set start bit (Bit 31 is set in the HAL although there is no
	 * mention of it in the datasheet) */
	I2C1->CR2	 = (I2C1->CR2 & ~(0xFF<<16 | 0x01<<10 | 0x3FF<<0)) | ((numOfBytes & 0xFF)<<16 | (slaveAddress&0x7F)<<1 | 0x01<<13);
}


void Device::HardwareI2C1::
beginReceive(const std::uint8_t slaveAddress, const std::uint8_t* dataDest, const std::size_t numOfBytes) const
{
	/* Configure DMA Rx Channel */
	DMA1->IFCR			|= 0x01<<24;
	DMA1_Channel7->CNDTR = numOfBytes;
	DMA1_Channel7->CMAR	 = reinterpret_cast<std::uint32_t>(dataDest);
	DMA1_Channel7->CCR	|= (0x01<<3 | 0x01<<1 | 0x01<<0);

	/* Configure Interrupts and DMA usage */
	I2C1->CR1	|= (0x01<<15 | 0x01<<7 | 0x01<<6 | 0x01<<4);

	/* Configure Reception parameters */
	I2C1->CR2	 = (I2C1->CR2 & ~(0xFF<<16 | 0x01<<10 | 0x3FF<<0)) | ((numOfBytes & 0xFF)<<16 | 0x01<<13 | 0x01<<10 | (slaveAddress&0x7F)<<1);
}


/* Interrupt handler */
void Device::HardwareI2C1::
i2cEventHandler(void) const
{
	/* Called when the transfer is complete  */
	I2C1->ICR	|= I2C1->ISR;
}


void Device::HardwareI2C1::
i2cErrorHandler(void) const
{
	/* Called when an I2C error occured */
	I2C1->ICR	|= I2C1->ISR;

	/* Execute callback handler */
	writeCompleteHandler_(MiscStuff::ErrorCode::Error);
}


void Device::HardwareI2C1::
dmaTxHandler(void) const
{
	/* DMA 1 Channel 6
	 * Called when all bytes to send are transmitted or an error occured */
	/* Check if there is an error or the Transmission is done
	 */
	/* Temporarely store the data in the status register */
	std::uint32_t statusReg = DMA1->ISR;

	/* Clear interrupt source */
	DMA1->IFCR	|= (0x01<<20);

	/* Disable channel and interrupts */
	DMA1_Channel6->CCR	&= ~(0x01<<3 | 0x01<<1 | 0x01<<0);

	/* Disable I2C1 */
	I2C1->CR1	&= ~(0x01<<14 | 0x01<<7 | 0x01<<6 | 0x01<<4);

	/* Check which interrupt occured */
	if (statusReg & 0x01<<23) {
		/* Error */
		writeCompleteHandler_(MiscStuff::ErrorCode::Error);
	}
	else if (statusReg & 0x01<<21) {
		/* Transmission complete */
		writeCompleteHandler_(MiscStuff::ErrorCode::Success);
	}
}


void Device::HardwareI2C1::
dmaRxHandler(void) const
{
	/* DMA 1 Channel 7
	 * Called when all bytes to receive are transmitted or an error occured */
	/* Check if there is an error or the Reception is done
	 */
	/* Temporarely store the data in the status register */
	std::uint32_t statusReg = DMA1->ISR;

	/* Clear interrupt source */
	DMA1->IFCR	|= (0x01<<24);

	/* Disable channel and interrupts */
	DMA1_Channel7->CCR	&= ~(0x01<<3 | 0x01<<1 | 0x01<<0);

	/* Disable I2C1 */
	I2C1->CR1	&= ~(0x01<<15 | 0x01<<7 | 0x01<<6 | 0x01<<4);


	if (statusReg & 0x01<<27) {
		/* Error */
		readCompleteHandler_(MiscStuff::ErrorCode::Error);
	}
	else if (statusReg & 0x01<<25) {
		/* Reception complete */
		readCompleteHandler_(MiscStuff::ErrorCode::Success);
	}
}

