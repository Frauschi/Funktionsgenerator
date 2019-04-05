
#include <HardwarePWM.h>
#include <cstdint>
#include <functional>
#include "stm32l476xx.h"

//---------------------------------------------------------------------------------------
//--------------------- Implementation of Class 'HardwarePWM' ---------------------------

Device::HardwarePWM::
HardwarePWM(TIM_TypeDef* const timer_base, GPIO_TypeDef* const gpio_base, const uint8_t gpio_pin) :
	timerBase_(timer_base),
	channel_(0)
{
	/* Pulse Width Modulation for Display Background Light
	 *
	 *	PWM1 green	PC8 TIM3 CH3
	 *	alternative		TIM8 CH3
	 *
	 *
	 *	PWM2 red 	PC9 TIM8 CH4
	 *	alternative		TIM3 CH4
	 *
	 */

	/* Enable clock for GPIO port */
	std::uint8_t bitPos = (reinterpret_cast<std::uint32_t>(gpio_base) - AHB2PERIPH_BASE)>>10;
	RCC->AHB2ENR		|= 0x01<<bitPos;

	/* Enable clock for Timer */
	if(timerBase_ == TIM8)
	{
		RCC->APB2ENR		|= 0x01<<13;	//Enable Timer 8
	}
	else
	{
		bitPos = (reinterpret_cast<std::uint32_t>(timer_base) - APB1PERIPH_BASE)>>10;
		RCC->APB1ENR1		|= 0x01<<bitPos;
	}

	/* Configure GPIO pin */
	gpio_base->MODER	 = (gpio_base->MODER & ~(0x03<<(2*gpio_pin))) | (0x02<<(2*gpio_pin));	/* Alternate function mode */
	gpio_base->OTYPER 	&= ~(0x01<<gpio_pin);		/* Output push pull */
	gpio_base->OSPEEDR	|= 0x03<<(2*gpio_pin);		/* Very high speed */
	gpio_base->PUPDR	&= ~(0x03<<(2*gpio_pin));	/* No pull-up / pull-down */

	/* Set alternate function */
	std::uint8_t alternateFunction = (timer_base == TIM8) ? 3 : 2; // 2 for TIM1, TIM2, TIM4, TIM5; 3 for TIM8
	if (gpio_pin <= 7) {
		gpio_base->AFR[0] = (gpio_base->AFR[0] & ~(0x0F<<(gpio_pin*4))) | alternateFunction<<(gpio_pin*4);
	}
	else {
		gpio_base->AFR[1] = (gpio_base->AFR[1] & ~(0x0F<<((gpio_pin-8)*4))) | alternateFunction<<((gpio_pin-8)*4);
	}


	/* Configure Timer */
	timerBase_->CR1		&= ~(0x01<<0);

	timerBase_->CR1 	|= (0x01<<7); // | 0x01<<4); // | 0x01<<2);
	//timerBase_->DIER 	&= ~(0x01<<0);
	if(timerBase_ == TIM8)
	{
		timerBase_->BDTR	|= 0x01<<14;	//Automatic Output enable
	}

	/* Check which channel to use */
	if (gpio_base == GPIOA) {
		if (gpio_pin <= 3) {
			channel_ = gpio_pin + 1;
		}
		else if ((gpio_pin == 6) || (gpio_pin == 7)) {
			channel_ = gpio_pin - 5;
		}
	}
	else if (gpio_base == GPIOB) {
		if ((gpio_pin == 4) || (gpio_pin == 6)) {
			channel_ = 1;
		}
		else if ((gpio_pin == 5) || (gpio_pin == 7)) {
			channel_ = 2;
		}
		else if ((gpio_pin == 0) || (gpio_pin == 8)) {
			channel_ = 3;
		}
		else if ((gpio_pin == 1) || (gpio_pin == 9)) {
			channel_ = 4;
		}
	}
	else if (gpio_base == GPIOC) {
		if ((gpio_pin >= 6) && (gpio_pin <= 9)) {
			channel_ = gpio_pin - 5;
		}
	}
	else if (gpio_base == GPIOD) {
		if (gpio_pin >= 12) {
			channel_ = gpio_pin - 11;
		}
	}
	else if (gpio_base == GPIOE) {
		if ((gpio_pin >= 3) && (gpio_pin <= 6)) {
			channel_ = gpio_pin - 2;
		}
	}
	else if (gpio_base == GPIOF) {
		if ((gpio_pin >= 6) && (gpio_pin <= 9)) {
			channel_ = gpio_pin - 5;
		}
	}

	/* Configure channel settings */
	if ((channel_ == 1) || (channel_ == 2)) {
		timerBase_->CCMR1	= (timerBase_->CCMR1 & ~(0xFF<<((channel_-1)*8))) | (0x01<<(((channel_-1)*8)+3) | 0x06<<(((channel_-1)*8)+4));
	}
	else if ((channel_ == 3) || (channel_ == 4)) {
		timerBase_->CCMR2   = (timerBase_->CCMR2 & ~(0xFF<<((channel_-3)*8))) | (0x01<<(((channel_-3)*8)+3) | 0x06<<(((channel_-3)*8)+4));
	}

	/* Set max timer value */
	timerBase_->ARR	 = 0xFFFF;

	/* Set duty cycle to 50% (default) */
	setDutyCycle(50);
}

Device::HardwarePWM::
~HardwarePWM(void)
{

}


void Device::HardwarePWM::
enable(void) const
{
	/* Disable timer */
	timerBase_->CR1		&= ~(0x01<<0);

	/* Enable output */
	timerBase_->CCER	|= (0x01<<((channel_-1)*4));

	/* Create an update event to use the new settings */
	timerBase_->EGR		|= 0x01<<0;

	/* Enable timer */
	timerBase_->CR1		|= 0x01<<0;
}


void Device::HardwarePWM::
disable(void) const
{
	/* Disable timer */
	timerBase_->CR1		&= ~(0x01<<0);

	/* Disable output */
	timerBase_->CCER	&= ~(0x01<<((channel_-1)*4));

	/* Create an update event to use the new settings */
	timerBase_->EGR		|= 0x01<<0;

	/* Enable timer */
	timerBase_->CR1		|= 0x01<<0;
}


void Device::HardwarePWM::
setDutyCycle(const std::uint8_t percent) const
{
	/* Disable timer */
	timerBase_->CR1		&= ~(0x01<<0);

	/* Update settings for new duty cycle */
	if (channel_ == 1) {
		timerBase_->CCR1 = ((0xFFFF * percent) / 100);
	}
	else if (channel_ == 2) {
		timerBase_->CCR2 = ((0xFFFF * percent) / 100);
	}
	else if (channel_ == 3) {
		timerBase_->CCR3 = ((0xFFFF * percent) / 100);
	}
	else if (channel_ == 4) {
		timerBase_->CCR4 = ((0xFFFF * percent) / 100);
	}

	/* Create an update event to use the new settings */
	timerBase_->EGR		|= 0x01<<0;

	/* Enable timer */
	timerBase_->CR1		|= 0x01<<0;
}

