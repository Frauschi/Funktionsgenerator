#ifndef HARDWAREENCODER_H_
#define HARDWAREENCODER_H_

#include <cstdint>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareEncoder.h"
#include "HardwareCore.h"


namespace Device
{

class HardwareEncoder
{

public:

	typedef std::function<void (void)> TOpCompleteHandler;

	//Constructor
	explicit HardwareEncoder(TIM_TypeDef* timerBase);

	//Destructor
	~HardwareEncoder();

	//Set Callback functions
	template <typename TFunc>
	void setLeftCompleteHandler(TFunc&& func) const
	{
		turnLeftHandler_ = std::forward<TFunc>(func);
	}

	template <typename TFunc>
	void setRightCompleteHandler(TFunc&& func) const
	{
		turnRightHandler_ = std::forward<TFunc>(func);
	}

	void enableEncoder(void) const;
	void disableEncoder(void) const;

private:

	TIM_TypeDef* const timerBase_;

	//Storage for Callback functions
	mutable TOpCompleteHandler	turnLeftHandler_;
	mutable TOpCompleteHandler	turnRightHandler_;

	//Interrupt Handler
	void encoderHandler(void) const;

};

}	/* Namespace Driver */

#endif	/*HardwareEncoder_H_*/
