
#include <cstdint>
#include <array>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareCore.h"



#define VECT_TAB_OFFSET	0x00
#define HSE_VALUE    	((uint32_t)8000000) /* Value of the External oscillator in Hz */
#define MSI_VALUE    	((uint32_t)4000000) /* Value of the Internal oscillator in Hz */
#define HSI_VALUE    	((uint32_t)16000000) /* Value of the Internal oscillator in Hz */


//---------------------------------------------------------------------------------------
//--------------------- Implementation of class 'InterruptMgr' --------------------------

// Initialize the only instance of the Interrupt Manager
const Device::InterruptMgr Device::InterruptMgr::theInterruptMgr_;


Device::InterruptMgr::
InterruptMgr()
{
	/* Set all callbacks in the handler arrays to invalid */
	for (std::size_t i = 0; i < Device::InterruptId::IntCount; i++) {
		handlerArray_[i] = nullptr;
	}

	for (std::size_t i = 0; i < Device::GpioIntId::NoInterrupt; i++) {
		gpioHandlerArray_[i] = {0, nullptr, false};
	}

	/* Add gpioInterrupt handler */
	addHandlerForInterrupt(Device::InterruptId::EXTI_Int, [this]() { this->gpioInterruptHandler(); });
}

void Device::InterruptMgr::
enableInterrupt(InterruptId const id) const
{
	switch (id) {
	case I2C1_EVInt:
		NVIC_EnableIRQ(I2C1_EV_IRQn);
		break;
	case I2C1_ERInt:
		NVIC_EnableIRQ(I2C1_ER_IRQn);
		break;
	case Timer2Int:
		NVIC_EnableIRQ(TIM2_IRQn);
		break;
	case DMA1_CH6Int:
		NVIC_EnableIRQ(DMA1_Channel6_IRQn);
		break;
	case DMA1_CH7Int:
		NVIC_EnableIRQ(DMA1_Channel7_IRQn);
		break;
	case SPI1_Int:
		NVIC_EnableIRQ(SPI1_IRQn);
		break;
	case DMA1_CH2Int:
		NVIC_EnableIRQ(DMA1_Channel2_IRQn);
		break;
	case DMA1_CH3Int:
		NVIC_EnableIRQ(DMA1_Channel3_IRQn);
		break;
	case SPI2_Int:
		NVIC_EnableIRQ(SPI2_IRQn);
		break;
	case DMA1_CH4Int:
		NVIC_EnableIRQ(DMA1_Channel4_IRQn);
		break;
	case DMA1_CH5Int:
		NVIC_EnableIRQ(DMA1_Channel5_IRQn);
		break;
	case EncoderTimer_Int:
		NVIC_EnableIRQ(TIM3_IRQn);
		break;
	case EXTI_Int:
		NVIC_EnableIRQ(EXTI0_IRQn);
		NVIC_EnableIRQ(EXTI1_IRQn);
		NVIC_EnableIRQ(EXTI2_IRQn);
		NVIC_EnableIRQ(EXTI3_IRQn);
		NVIC_EnableIRQ(EXTI4_IRQn);
		NVIC_EnableIRQ(EXTI9_5_IRQn);
		NVIC_EnableIRQ(EXTI15_10_IRQn);

		break;
	default:
		break;
	}

}


void Device::InterruptMgr::
disableInterrupt(InterruptId const id) const
{
	switch (id) {
	case I2C1_EVInt:
		NVIC_DisableIRQ(I2C1_EV_IRQn);
		break;
	case I2C1_ERInt:
		NVIC_DisableIRQ(I2C1_ER_IRQn);
		break;
	case Timer2Int:
		NVIC_DisableIRQ(TIM2_IRQn);
		break;
	case DMA1_CH6Int:
		NVIC_DisableIRQ(DMA1_Channel6_IRQn);
		break;
	case DMA1_CH7Int:
		NVIC_DisableIRQ(DMA1_Channel7_IRQn);
		break;
	case SPI1_Int:
		NVIC_DisableIRQ(SPI1_IRQn);
		break;
	case DMA1_CH2Int:
		NVIC_DisableIRQ(DMA1_Channel2_IRQn);
		break;
	case DMA1_CH3Int:
		NVIC_DisableIRQ(DMA1_Channel3_IRQn);
		break;
	case SPI2_Int:
		NVIC_DisableIRQ(SPI2_IRQn);
		break;
	case DMA1_CH4Int:
		NVIC_DisableIRQ(DMA1_Channel4_IRQn);
		break;
	case DMA1_CH5Int:
		NVIC_DisableIRQ(DMA1_Channel5_IRQn);
		break;
	case EncoderTimer_Int:
		NVIC_DisableIRQ(TIM3_IRQn);
		break;
	case EXTI_Int:
		NVIC_DisableIRQ(EXTI0_IRQn);
		NVIC_DisableIRQ(EXTI1_IRQn);
		NVIC_DisableIRQ(EXTI2_IRQn);
		NVIC_DisableIRQ(EXTI3_IRQn);
		NVIC_DisableIRQ(EXTI4_IRQn);
		NVIC_DisableIRQ(EXTI9_5_IRQn);
		NVIC_DisableIRQ(EXTI15_10_IRQn);
		break;
	default:
		break;
	}

}

void Device::InterruptMgr::
enableGpioIntForEdge(GpioPin const pin, Edge const edge) const
{
	/* Configure EXTI registers */
	EXTI->IMR1	|= 0x01<<(pin/8);

	if (edge == Edge::Falling || edge == Edge::Both)
		EXTI->FTSR1	|= 0x01<<(pin/8);
	else
		EXTI->FTSR1	&= ~(0x01<<(pin/8));

	if (edge == Edge::Rising || edge == Edge::Both)
		EXTI->RTSR1	|= 0x01<<(pin/8);
	else
		EXTI->RTSR1	&= ~(0x01<<(pin/8));


	/* Configure SYSCFG register */
	SYSCFG->EXTICR[pin/32]  = (SYSCFG->EXTICR[pin/32] & ~(0x0F<<(((pin/8)-((pin/32)*4))*4))) | ((pin%8)<<(((pin/8)-((pin/32)*4))*4));
}


void Device::InterruptMgr::
handleInterrupt(InterruptId const id) const
{
	/* Execute callback if valid */
	if (handlerArray_[id])
		handlerArray_[id]();
}

void Device::InterruptMgr::
activateEdgeInterrupt(GpioIntId const id) const
{
	gpioHandlerArray_[id].active = true;
}

void Device::InterruptMgr::
deactivateEdgeInterrupt(GpioIntId const id) const
{
	gpioHandlerArray_[id].active = false;
}

void Device::InterruptMgr::
gpioInterruptHandler(void) const
{
	/* Get interrupt source */
	std::uint32_t intSrc = EXTI->PR1 & 0xFFFF;
	std::uint8_t line = 0;
	while ((intSrc & 0x01) == 0x00) {
		intSrc >>= 1;
		line += 1;
	}

	/* Clear pending interrupt */
	EXTI->PR1	|= 0x01<<line;

	for (std::size_t i = 0; i < Device::GpioIntId::NoInterrupt; i++) {
		if (gpioHandlerArray_[i].line == line) {
			/* Execute callback if valid */
			if (gpioHandlerArray_[i].callback && gpioHandlerArray_[i].active)
				gpioHandlerArray_[i].callback();

			break;
		}
	}
}


//---------------------------------------------------------------------------------------
//------------------------- Implementation of class 'Core' ------------------------------

Device::Core::
Core()
{
}


Device::Core::
~Core()
{
}


std::uint32_t Device::Core::
systemCoreClock(void)
{
	std::uint32_t tmp = 0, msirange = 0, pllvco = 0, pllr = 2, pllsource = 0, pllm = 2, retVal = 0;

	std::uint8_t  AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
	//std::uint8_t  APBPrescTable[8] =  {0, 0, 0, 0, 1, 2, 3, 4};
	std::uint32_t MSIRangeTable[12] = {100000, 200000, 400000, 800000, 1000000, 2000000,
		                                      4000000, 8000000, 16000000, 24000000, 32000000, 48000000};

	/* Get MSI Range frequency--------------------------------------------------*/
	if((RCC->CR & RCC_CR_MSIRGSEL) == 0) { /* MSISRANGE from RCC_CSR applies */
		msirange = (RCC->CSR & RCC_CSR_MSISRANGE) >> 8;
	}
	else { /* MSIRANGE from RCC_CR applies */
		msirange = (RCC->CR & RCC_CR_MSIRANGE) >> 4;
	}

	/*MSI frequency range in HZ*/
	msirange = MSIRangeTable[msirange];

	/* Get SYSCLK source -------------------------------------------------------*/
	switch (RCC->CFGR & RCC_CFGR_SWS) {
	case 0x00:  /* MSI used as system clock source */
		retVal = msirange;
		break;

	case 0x04:  /* HSI used as system clock source */
		retVal = HSI_VALUE;
		break;

	case 0x08:  /* HSE used as system clock source */
		retVal = HSE_VALUE;
		break;

	case 0x0C:  /* PLL used as system clock  source */
		/* PLL_VCO = (HSE_VALUE or HSI_VALUE or MSI_VALUE/ PLLM) * PLLN
		 * SYSCLK = PLL_VCO / PLLR
		 */
		pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC);
		pllm = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> 4) + 1 ;

		switch (pllsource) {
		case 0x02:  /* HSI used as PLL clock source */
			pllvco = (HSI_VALUE / pllm);
			break;

		case 0x03:  /* HSE used as PLL clock source */
			pllvco = (HSE_VALUE / pllm);
			break;

		default:    /* MSI used as PLL clock source */
			pllvco = (msirange / pllm);
			break;
		}
		pllvco = pllvco * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 8);
		pllr = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLR) >> 25) + 1) * 2;
		retVal = pllvco/pllr;
		break;

	default:
		retVal = msirange;
		break;
	}

	/* Compute HCLK clock frequency --------------------------------------------*/
	/* Get HCLK prescaler */
	tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
	/* HCLK clock frequency */
	retVal >>= tmp;

	return retVal;
}


void Device::Core::
setSystemCoreClock(void)
{
	/* Steps needed to set the system clock to 80MHz:
	 * 		- Set the voltage regulator range to 1 (1,2V)
	 * 		- Increase the wait states for the FLASH to 4
	 * 		- Activate HSI
	 * 		- Configure PLL
	 * 		- Activate PLL and set it as main clock source
	 * 		- Disable MSI
	 *
	 * 	All divider are set to 1. Therefore all bus clocks (AHB, APB) are also 80MHz!
	 */

	/* Enable clock for PWR and SYSCFG cicuitry */
	RCC->APB1ENR1	|= 0x01<<28;
	RCC->APB2ENR	|= 0x01<<0;

	/* Check if PLL is already clock source */
	if ((RCC->CFGR & 0x03<<2) != 0x03<<2) {
		/* Configure the main internal regulator output voltage */
		PWR->CR1 = 0x00000200;

		/* Wait until voltage is stable */
		while ((PWR->SR2 & 0x01<<10) == 0x01<<10) {}

		/* Set Flash latency to 4 clock cycles due to the much higher clock */
		FLASH->ACR		|= 0x04<<0;

		/* Enable HSI */
		RCC->CR		|= 0x01<<8;

		/* Wait for HSI to power up */
		while ((RCC->CR & 0x01<<10) == 0) { }

		/* Disable PLL */
		RCC->CR		&= ~(0x01<<24);

		/* Wait for PLL to be unlocked */
		while ((RCC->CR & 0x01<<25) != 0) { }

		/* Set PLL Config Register */
		RCC->PLLCFGR	 = 0;
		RCC->PLLCFGR	|= 0x02<<0; 	/* HSI as PLL input */
		RCC->PLLCFGR	|= 0x00<<4; 	/* PLLM = 1 */
		RCC->PLLCFGR	|= 0x0A<<8; 	/* PLLN = 10 */
		RCC->PLLCFGR	|= 0x00<<16; 	/* PLLSAI3CLK output disable */
		RCC->PLLCFGR	|= 0x00<<17;	/* PLLP = 7 */
		RCC->PLLCFGR	|= 0x00<<20;	/* PLL48M1CLK output disable */
		RCC->PLLCFGR	|= 0x00<<21;	/* PLLQ = 2 */
		RCC->PLLCFGR	|= 0x00<<25;	/* PLLR = 2 */
		RCC->PLLCFGR	|= 0x00<<27; 	/* PLLSAI3CLK is controlled by the bit PLLP */

		/* Enable PLL */
		RCC->CR		|= 0x01<<24;

		/* PLLCLK output enable */
		RCC->PLLCFGR	|= 0x01<<24;

		/* Wait for PLL to be locked */
		while ((RCC->CR & 0x01<<25) == 0) { }

		/* Set Config Register: Set PLL output as Clock Source and all dividers to 1 */
		RCC->CFGR 	 = 0x00008003;

		/* Wait until clock source is set properly */
		while ((RCC->CFGR & 0x03<<2) != 0x03<<2) { }

		/* Disable MSI */
		RCC->CR		&= ~(0x01<<0);

		/* Wait until MSI is off */
		while (RCC->CR & 0x01<<1) { }

		/* Set I2C1 clock source to HSI16 */
		RCC->CCIPR		|= 0x02<<12;
	}
}


extern "C" void SystemInit(void)
{
	/* FPU settings ------------------------------------------------------------*/
	#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
		SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
	#endif


	/* Set system clock --------------------------------------------------------*/
	Device::Core::setSystemCoreClock();

	/* Configure the Vector Table location add offset address ------------------*/
	#ifdef VECT_TAB_SRAM
		SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
	#else
		SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */ // @suppress("Field cannot be resolved")
	#endif
}

//---------------------------------------------------------------------------------------
//--------------------------- Interrupt Service Routines --------------------------------

// I2C1 (event interrupt)
extern "C" __attribute__ ((interrupt ("IRQ")))
void I2C1_EV_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::I2C1_EVInt);
}


// I2C1 (event interrupt)
extern "C" __attribute__ ((interrupt ("IRQ")))
void I2C1_ER_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::I2C1_ERInt);
}


// Timer2 (Wait Timer)
extern "C" __attribute__ ((interrupt ("IRQ")))
void TIM2_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::Timer2Int);
}


// DMA 1 Channel 6
extern "C" __attribute__ ((interrupt ("IRQ")))
void DMA1_CH6_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::DMA1_CH6Int);
}


// DMA 1 Channel 7
extern "C" __attribute__ ((interrupt ("IRQ")))
void DMA1_CH7_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::DMA1_CH7Int);
}


// SPI 1
extern "C" __attribute__ ((interrupt ("IRQ")))
void SPI1_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::SPI1_Int);
}

// DMA 1 Channel 2
extern "C" __attribute__ ((interrupt ("IRQ")))
void DMA1_CH2_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::DMA1_CH2Int);
}

// DMA 1 Channel 3
extern "C" __attribute__ ((interrupt ("IRQ")))
void DMA1_CH3_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::DMA1_CH3Int);
}

// SPI 2
extern "C" __attribute__ ((interrupt ("IRQ")))
void SPI2_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::SPI2_Int);
}

// DMA 1 Channel 4
extern "C" __attribute__ ((interrupt ("IRQ")))
void DMA1_CH4_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::DMA1_CH4Int);
}

// DMA 1 Channel 5
extern "C" __attribute__ ((interrupt ("IRQ")))
void DMA1_CH5_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::DMA1_CH5Int);
}


// TIM3 Interrupt (Encoder)
extern "C" __attribute__ ((interrupt ("IRQ")))
void TIM3_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EncoderTimer_Int);
}


// EXTI
extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI0_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}

extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI1_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}

extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI2_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}

extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI3_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}

extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI4_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}

extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI9_5_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}

extern "C" __attribute__ ((interrupt ("IRQ")))
void EXTI15_10_IRQHandler(void)
{
	Device::InterruptMgr::reference().handleInterrupt(Device::InterruptId::EXTI_Int);
}


