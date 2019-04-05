
#ifndef HARDWARETIMER_H_
#define HARDWARETIMER_H_


#include <functional>
#include <cstdint>
#include <chrono>
#include "stm32l476xx.h"


namespace Device
{

class HardwareTimer
{
public:

	/* Typedefs for cleaner code */
	typedef	std::chrono::duration<std::int64_t,	std::milli> TimeUnitDuration;
	typedef std::function<void (void)> TCallbackHandler;


	/* Constructor */
	HardwareTimer(TIM_TypeDef* base, const std::uint32_t coreClock);

	~HardwareTimer();

	/* Start the timer with the given time */
	void start(const TimeUnitDuration::rep count) const;

	/* Stop timer */
	void stop(void) const ;

	/* Returns time the timer was running til it was stopped */
	TimeUnitDuration getWaitedTime(void) const ;

	/* Set callback handler */
	template <typename TFunc>
	void setCallbackHandler(TFunc&& func) const
	{
		callbackHandler_ = std::forward<TFunc>(func);
	}


private:

	TIM_TypeDef* const base_;
	std::uint32_t const unitScale_;
	mutable TCallbackHandler callbackHandler_;

	inline void setTimerEnabled(bool enabled) const;

	inline void setInterruptEnabled(bool enabled) const ;

	void interruptHandler(void) const ;

	static const std::int32_t scaleCalibrationValue = 4000;

};


} /* namespace device */

#endif /* HARDWARETIMER_H_ */
