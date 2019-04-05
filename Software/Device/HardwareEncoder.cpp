#include <cstdint>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareEncoder.h"
#include "HardwareCore.h"

Device::HardwareEncoder::
HardwareEncoder(TIM_TypeDef* timerBase) :
timerBase_(timerBase)
{
	/* Encoder:
	 *	using Timer 1
	 * 		TIM1_CH1	PA8
	 * 		TIM1_CH2	PA9
	 *
	 *	using Timer 3
	 * 		TIM3_CH1	PB4
	 * 		TIM3_CH2	PB5
	 *
	 */

	if(timerBase_ == TIM1)
	{
		/* Enable Timer 1 */
		RCC->APB2ENR 	|= 0x01<<11;

		/* Enable GPIOA */
		RCC->AHB2ENR	|= 0x01<<0;

		/* Configure GPIOA Port */
		GPIOA->MODER	&= ~(0x03<<16 | 0x03<<18);
		GPIOA->MODER	|= (0x02<<16 | 0x02<<18);
		GPIOA->PUPDR	&= ~(0x03<<16 | 0x03<<18);
		GPIOA->AFR[1]	|= (0x01<<0| 0x01<<4);		//AF1 = TIM1

	}
	else if(timerBase_ == TIM3)
	{
		/* Enable GPIOB */
		RCC->AHB2ENR	|= 0x01<<1;

		/* Enable Timer 3 */
		RCC->APB1ENR1 	|= 0x01<<1;

		/* Configure GPIOB Port */
		GPIOB->AFR[0]	 = (GPIOB->AFR[0] & ~(0x0F<<16 | 0x0F<<20)) | (0x02<<16| 0x02<<20);		//AF2 = TIM3
		GPIOB->MODER	 = (GPIOB->MODER & ~(0x03<<8 | 0x03<<10)) | (0x02<<8 | 0x02<<10);
		GPIOB->PUPDR	&= ~(0x03<<8 | 0x03<<10);
	}

	/* Disable Timer 3 */
	timerBase_->CR1		&= ~(0x01<<0);

	/* Configure TIM3 */
	timerBase_->SMCR		|= 0x03<<0;				//Encoder Mode 3
	timerBase_->CCMR1		|= (0x01<<8 | 0x01<<0); //CC1 und CC2 input
	timerBase_->CCER		|= (0x01<<1 | 0x01<<5);

	timerBase_->ARR		= 1;

	timerBase_->DIER		|= (0x01<<0);

	/* Enable Interrupt and add Handler */
	Device::InterruptMgr::reference().addHandlerForInterrupt(Device::InterruptId::EncoderTimer_Int, [this]() { encoderHandler(); });
}


Device::HardwareEncoder::
~HardwareEncoder()
{
	Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::EncoderTimer_Int);

	if(timerBase_ == TIM1)
	{
		/* Disable Timer 1*/
		RCC->APB2ENR 	&= ~(0x01<<11);
	}
	else if(timerBase_ == TIM3)
	{
		/* Disable Timer 3*/
		RCC->APB1ENR1 	&= ~(0x01<<1);
	}
}


void Device::HardwareEncoder::
enableEncoder(void) const
{
	/* Enable timer */
	timerBase_->CR1		|= 0x01<<0;
}


void Device::HardwareEncoder::
disableEncoder(void) const
{
	/* Disable timer */
	timerBase_->CR1		&= ~(0x01<<0);
}


void Device::HardwareEncoder::
encoderHandler(void) const
{
	/* Reset interrupt */
	timerBase_->SR	&= ~(0x01<<0);

	/* Check direction and call proper callback */
	if(timerBase_->CR1 & (0x01<<4)) //Encoder was turned left
	{
		if(turnRightHandler_)
		{
			turnRightHandler_();
		}
	}
	else //Encoder was turned right
	{
		if(turnLeftHandler_)
		{
			turnLeftHandler_();
		}
	}
}

