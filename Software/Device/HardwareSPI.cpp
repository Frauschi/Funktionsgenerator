
#include <HardwareSPI.h>
#include <cstdint>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareCore.h"

//---------------------------------------------------------------------------------------
// -------------------- Implementation of Class 'HardwareSPI' --------------------------


Device::HardwareSPI::
HardwareSPI(SPI_TypeDef* base) :
	spiBase_(base),
	currentClkPhase_(ClockPhase::FirstEdge)
{
	/* Local variables for respective pins */
	uint8_t pinSCK_ = 0;
	uint8_t pinMOSI_ = 0;
	uint8_t pinMISO_ = 0;

	DMA_Request_TypeDef* dmaCSELR_;

	/*SPI clk divider
	 *
	 * 	000: fPCLK/2
	 *	001: fPCLK/4
	 *	010: fPCLK/8
	 *	011: fPCLK/16
	 *	100: fPCLK/32
	 *	101: fPCLK/64
	 *  110: fPCLK/128
	 *	111: fPCLK/256
	 */
	uint8_t SPI_fclk_div = 0;

	if(spiBase_ == SPI1) {
		/* SPI1:
		 * 		GPIO	(PA4		SPI1_NSS)
		 * 				PA5			SPI1_SCK
		 * 				PA6			SPI1_MISO
		 * 				PA7			SPI1_MOSI
		 *
		 * 		DMA 1	Channel 2 	SPI1_RX
		 * 				Channel 3	SPI1_TX
		 */

		/* Enable Clock for GPIOA */
		RCC->AHB2ENR		|= 0x01<<0;
		/* Enable Clock for DMA1 */
		RCC->AHB1ENR		|= 0x01<<0;
		/* Enable Clock for SPI1 */
		RCC->APB2ENR 		|= 0x01<<12;

		/* Set Hardware Devices for SPI1 */
		gpioBase_ = GPIOA;
		pinSCK_ = 5;
		pinMOSI_ = 7;
		pinMISO_ = 6;

		dmaBase_ = DMA1;
		dmaRX_ = DMA1_Channel2;
		dmaTX_ = DMA1_Channel3;
		dmaChRX_ = 2;
		dmaChTX_ = 3;

		SPI_fclk_div = 4;


		/* Configure SPI1 Interrupts */
		Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::SPI1_Int, [this]() { this->spiHandler(); });

		/* Configure DMA interrupts */
		Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::DMA1_CH3Int, [this]() { this->dmaTxHandler(); });
		Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::DMA1_CH2Int, [this]() { this->dmaRxHandler(); });

		dmaCSELR_ = DMA1_CSELR;
	}
	else if(spiBase_ == SPI2) {
		/* SPI2:
		 *
		 * 		GPIO	(PB12		SPI2_NSS)
		 * 				PB13		SPI2_SCK
		 * 				PB14		SPI2_MISO
		 * 				PB15		SPI2_MOSI
		 *
		 * 		DMA 1	Channel 4	SPI2_RX
		 * 				Channel 5	SPI2_TX
		 */

		/* Enable Clock for GPIOB */
		RCC->AHB2ENR		|= 0x01<<1;
		/* Enable Clock for DMA1 */
		RCC->AHB1ENR		|= 0x01<<0;
		/* Enable Clock for SPI2 */
		RCC->APB1ENR1 		|= 0x01<<14;

		/* Set Hardware Devices for SPI1 */
		gpioBase_ = GPIOB;
		pinSCK_ = 13;
		pinMOSI_ = 15;
		pinMISO_ = 14;

		dmaBase_ = DMA1;
		dmaRX_ = DMA1_Channel4;
		dmaTX_ = DMA1_Channel5;
		dmaChRX_ = 4;
		dmaChTX_ = 5;

		SPI_fclk_div = 4;

		/* Configure SPI2 Interrupts */
		Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::SPI2_Int, [this]() { this->spiHandler(); });

		/* Configure DMA interrupts */
		Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::DMA1_CH5Int, [this]() { this->dmaTxHandler(); });
		Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::DMA1_CH4Int, [this]() { this->dmaRxHandler(); });

		dmaCSELR_ = DMA1_CSELR;
	}

	/* From Here on initialisation for all SPIs is equal */
	/* Configure GPIO */
	gpioBase_->MODER		&= ~(0x03<<(pinSCK_*2) | 0x03<<(pinMOSI_*2) | 0x03<<(pinMISO_*2));
	gpioBase_->MODER		|= (0x02<<(pinSCK_*2) | 0x02<<(pinMOSI_*2) | 0x02<<(pinMISO_*2));	//Set Alternate function
	gpioBase_->OTYPER		&= ~(0x01<<(pinSCK_) | 0x01<<(pinMOSI_) | 0x01<<(pinMISO_));		//Push-Pull Configuration
	gpioBase_->OSPEEDR		|= (0x03<<(pinSCK_*2) | 0x03<<(pinMOSI_*2) | 0x03<<(pinMISO_*2));	//Output Speed Very High
	gpioBase_->PUPDR		&= ~(0x03<<(pinSCK_*2) | 0x03<<(pinMOSI_*2) | 0x03<<(pinMISO_*2));	//No Pull Resistors

	if(pinMOSI_ < 8) {
		gpioBase_->AFR[0]	|= 0x05<<(pinMOSI_*4);		//Select AF5 (SPI)
	}
	else {
		gpioBase_->AFR[1]	|= 0x05<<((pinMOSI_-8)*4);
	}

	if(pinSCK_ < 8) {
		gpioBase_->AFR[0]	|= 0x05<<(pinSCK_ *4);		//Select AF5 (SPI)
	}
	else {
		gpioBase_->AFR[1]	|= 0x05<<((pinSCK_ -8)*4);
	}

	if(pinMISO_ < 8) {
		gpioBase_->AFR[0]	|= 0x05<<(pinMISO_*4);		//Select AF5 (SPI)
	}
	else {
		gpioBase_->AFR[1]	|= 0x05<<((pinMISO_-8)*4);
	}

	/* Configuration DMA */
	std::uint8_t MemPerphSize = 0x00;

	switch(sizeof(DataType)) {
		case 1:		MemPerphSize = 0x00; 	break;
		case 2:		MemPerphSize = 0x01; 	break;
		case 4:		MemPerphSize = 0x02; 	break;
		default:	MemPerphSize = 0x00; 	break;
	}

	/*	Configure DMA RX Channel
	 * Medium Prio,
	 * 8, 16, 32 Bit Memory depending on size of Datatype,
	 * 8, 16, 32 Bit Peripherial depending on size of Datatype,
	 * Memory increment mode,
	 * Transfer error interrupt
	 * Transfer complete interrupt
	 * Read from Peripherial
	 */
	dmaRX_->CCR	|= (0x01<<12 | MemPerphSize<<10 | MemPerphSize<<8 | 0x01<<7);
	dmaRX_->CPAR = reinterpret_cast<std::uint32_t>(&spiBase_->DR);							//SPI Data Register is Source Adress for DMA

	/*	Configure DMA TX Channel
	 * Medium Prio,
	 * 8, 16, 32 Bit Memory depending on size of Datatype,
	 * 8, 16, 32 Bit Peripherial depending on size of Datatype,
	 * Memory increment mode,
	 * Transfer error interrupt
	 * Transfer complete interrupt
	 * Read from Memory
	 */
	dmaTX_->CCR	|= (0x01<<12 | MemPerphSize<<10 | MemPerphSize<<8 | 0x01<<7 | 0x01<<4);
	dmaTX_->CPAR = reinterpret_cast<std::uint32_t>(&spiBase_->DR);						//SPI Data Register is Destination Adress for DMA

	/* Mapping fo the DMA to the corresponding SPI Pins */
//		dmaCSELR_ = (DMA_Request_TypeDef *) (dmaBase_ + 0x00A8U);
	dmaCSELR_->CSELR |= (0x01<<((dmaChRX_-1)*4) | 0x01<<((dmaChTX_-1)*4));

	/* Configuration SPI*/
	std::uint8_t DataSize = 0x00;

	switch(sizeof(DataType)) {
		case 1:		DataSize = 0x07; 	break;
		case 2:		DataSize = 0x0F; 	break;
		default:	DataSize = 0x07; 	break;
	}

	/*Disable SPI*/
	spiBase_->CR1		&= ~(0x01<<6);

	/* Configure SPI with
	 * Clk_idle = 1
	 * First Clock transition is first data capture edge
	 * Master Mode
	 * BaudRate = f_PCLK/128 -> 80MHz/128 = 625kHz
	 * Data MSB first
	 * Software Slave management (SSI Bit = 0)
	 *
	 * Data Size depending on size of DataType
	 * SPI Motorola Mode
	 * Error Interrupt Enable
	 * Tx Buffer Empty Interrupt Enable
	 * Rx Buffer not Empty Interrupt Enable
	 *
	 */
	spiBase_->CR1 		|= (SPI_fclk_div<<3 | 0x01<<2 | 0x01<<1);
	spiBase_->CR2 		|= (0x01<<12 | DataSize<<8 | 0x01<<2 | 0x01<<1 | 0x01<<0);

	/* Enable SPI */
	spiBase_->CR1		|= (0x01<<6);
}


Device::HardwareSPI::
~HardwareSPI()
{
	if(spiBase_ == SPI1) {
		//Disable Interrupts
		Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::SPI1_Int);
		Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::DMA1_CH2Int);
		Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::DMA1_CH3Int);

		/* Disable Clock for SPI1 */
		RCC->APB2ENR 	&= ~(0x01<<12);
	}
	else if(spiBase_ == SPI2) {
		//Disable Interrupts
		Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::SPI2_Int);
		Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::DMA1_CH4Int);
		Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::DMA1_CH5Int);

		/* Enable Clock for SPI2 */
		RCC->APB1ENR1 	&= ~(0x01<<14);
	}
}


void Device::HardwareSPI::
beginTransmit(const DataType* dataSrc, const std::size_t numOfBytes) const
{
	/* Clear errors of DMA */
	dmaBase_->IFCR	|= (0x01<<((dmaChTX_-1)*4));

	/* Configure DMA Tx Channel*/
	dmaTX_->CNDTR	 = numOfBytes;
	dmaTX_->CMAR	 = reinterpret_cast<std::uint32_t>(dataSrc);

	/* Enable DMA Tx Channel*/
	dmaTX_->CCR		|= (0x01<<3 | 0x01<<1 | 0x01<<0); /* Transfer error and Transfer complete interrupt enable */
}


void Device::HardwareSPI::
beginReceive(const DataType* dataDest, const std::size_t numOfBytes) const
{
	/* Clear errors of DMA */
	dmaBase_->IFCR	|= (0x01<<((dmaChRX_-1)*4));

	/* Configure DMA Rx Channel*/
	dmaRX_->CNDTR	 = numOfBytes;
	dmaRX_->CMAR	 = reinterpret_cast<std::uint32_t>(dataDest);

	/* Enable DMA Rx Channel*/
	dmaRX_->CCR		|= (0x01<<3 | 0x01<<1 | 0x01<<0); /* Transfer error and Transfer complete interrupt enable */
}


void Device::HardwareSPI::
setClockPhase(ClockPhase const phase) const
{
	if (phase != currentClkPhase_) {
		currentClkPhase_ = phase;

		/* Wait until SPI peripheral busy bit is cleared */
		while(spiBase_->SR & 0x01<<7) { }

		/*Disable SPI*/
		spiBase_->CR1		&= ~(0x01<<6);

		/* Change clock phase */
		if (phase == ClockPhase::FirstEdge) {
			spiBase_->CR1	&= ~(0x01<<0);
		}
		else if (phase == ClockPhase::SecondEdge) {
			spiBase_->CR1	|= 0x01<<0;
		}

		/* Enable SPI */
		spiBase_->CR1		|= (0x01<<6);
	}
}


void Device::HardwareSPI::
spiHandler(void) const
{


}


void Device::HardwareSPI::
dmaTxHandler(void) const
{
	/* Temporarely store the data in the status register */
	std::uint32_t statusReg = dmaBase_->ISR;

	/* Disable channel and interrupts */
	dmaTX_->CCR	&= ~(0x01<<3 | 0x01<<1 | 0x01<<0);

	/* Clear interrupt source */
	dmaBase_->IFCR	|= (0x01<<((dmaChTX_-1)*4));

	/* Wait until SPI peripheral busy bit is cleared */
	while(spiBase_->SR & 0x01<<7) { }

	if (statusReg & 0x01<<(((dmaChTX_-1)*4)+3)) {
		/* Error */
		writeCompleteHandler_(MiscStuff::ErrorCode::Error);
	}
	else if (statusReg & 0x01<<(((dmaChTX_-1)*4)+1)) {
		/* Transmission complete */
		writeCompleteHandler_(MiscStuff::ErrorCode::Success);
	}
}


void Device::HardwareSPI::
dmaRxHandler(void) const
{
	/* Temporarely store the data in the status register */
	std::uint32_t statusReg = dmaBase_->ISR;

	/* Disable channel and interrupts */
	dmaRX_->CCR	&= ~(0x01<<3 | 0x01<<1 | 0x01<<0);

	/* Clear interrupt source */
	dmaBase_->IFCR	|= (0x01<<((dmaChRX_-1)*4));

	if (statusReg & 0x01<<(((dmaChRX_-1)*4)+3)) {
		/* Error */
		writeCompleteHandler_(MiscStuff::ErrorCode::Error);
	}
	else if (statusReg & 0x01<<(((dmaChRX_-1)*4)+1)) {
		/* Reception complete */
		writeCompleteHandler_(MiscStuff::ErrorCode::Success);
	}
}

