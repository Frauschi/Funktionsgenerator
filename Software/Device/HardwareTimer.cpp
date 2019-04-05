
#include <functional>
#include <cstdint>
#include <chrono>
#include "stm32l476xx.h"
#include "HardwareCore.h"
#include "HardwareTimer.h"


//---------------------------------------------------------------------------------------
// ------------------- Implementation of Class 'HardwareTimer' --------------------------

Device::HardwareTimer::
HardwareTimer(TIM_TypeDef* base, const std::uint32_t coreClock) :
	base_(base),
	unitScale_(coreClock / TimeUnitDuration::period::den)
{
	using namespace Device;

	// activate Clock for Timer
	std::uint8_t bitPos = (reinterpret_cast<std::uint32_t>(base_) - APB1PERIPH_BASE)>>10;
	RCC->APB1ENR1	|= 0x01<<bitPos;

	// configure Timer registers
	base_->CR1		|= 0x01<<2;

	base_->DIER		&= ~(0x01<<0);
	base_->CR1		&= ~(0x01<<0);

	// configure Interrupt
	InterruptMgr::reference().addHandlerForInterrupt(InterruptId::Timer2Int, [this]() { this->interruptHandler(); });
}


Device::HardwareTimer::
~HardwareTimer()
{
	// Disable interrupt
	Device::InterruptMgr::reference().disableInterrupt(Device::InterruptId::Timer2Int);

	// Disable clock
	std::uint8_t bitPos = (reinterpret_cast<std::uint32_t>(base_) - APB1PERIPH_BASE)>>10;
	RCC->APB1ENR1	&= ~(0x01<<bitPos);
}


inline void Device::HardwareTimer::
setTimerEnabled(bool enabled) const
{
	if (enabled)
		base_->CR1	|= 0x01<<0;
	else
		base_->CR1	&= ~(0x01<<0);
}


inline void Device::HardwareTimer::
setInterruptEnabled(bool enabled) const
{
	if (enabled)
		base_->DIER	|= 0x01<<0;
	else
		base_->DIER	&= ~(0x01<<0);
}


void Device::HardwareTimer::
start(const TimeUnitDuration::rep count) const
{
	// count is time in milli seconds!

	// Reset CNT value of timer
	base_->CNT = 0;

	// Set prescaler and max count register
	// Due to the integer division, the calculation for max count gets very big, but otherwise
	// uncommon wait times will be wrong!
	if (count <= 30000) { // time <= 30 sec
		base_->PSC = 0x00;
		base_->ARR = unitScale_*count + scaleCalibrationValue;
	}
	else if (count > 30000 && count <= 3600000) { // 30 sec < time <= 1hr
		base_->PSC = 999;
		base_->ARR = (unitScale_ * (count / 1000)) + ((unitScale_ * (count % 1000)) / 1000) + scaleCalibrationValue;
	}
	else { // time > 1hr
		base_->PSC = 29999;
		base_->ARR = (unitScale_ * (count / 30000)) + ((unitScale_ * (count % 30000)) / 30000) + scaleCalibrationValue;
	}

	// generate update event to use new ARR and PSC values
	base_->EGR |= 0x01<<0;

	// start timer
	setInterruptEnabled(true);
	setTimerEnabled(true);
}


void Device::HardwareTimer::
stop(void) const
{
	// stop timer
	setInterruptEnabled(false);
	setTimerEnabled(false);
}



Device::HardwareTimer::TimeUnitDuration
Device::HardwareTimer::
getWaitedTime(void) const
{
	return TimeUnitDuration((base_->CNT * (base_->PSC + 1)) / unitScale_);
}


void Device::HardwareTimer::
interruptHandler(void) const
{
	// clear flag in status register
	base_->SR	&= ~(0x01<<0);

	// disable timer
	setInterruptEnabled(false);
	setTimerEnabled(false);

	// execute callback function
	callbackHandler_();
}

