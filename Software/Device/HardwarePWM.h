#ifndef HARDWAREPWM_H_
#define HARDWAREPWM_H_

#include <cstdint>
#include <functional>
#include "stm32l476xx.h"


namespace Device
{

class HardwarePWM
{
public:

	//Constructor
	HardwarePWM(TIM_TypeDef* const timer_base, GPIO_TypeDef* const gpio_base, const uint8_t gpio_pin);

	//Destructor
	~HardwarePWM(void);

	void enable(void) const;
	void disable(void) const;

	/* Set duty cycle
	 * Parameter is a number between 0 and 100 (duty cycle in percent) */
	void setDutyCycle(const std::uint8_t percent) const;


private:

	TIM_TypeDef* const timerBase_;
	std::uint8_t channel_;
};

} /* end namepsace Device */


#endif

