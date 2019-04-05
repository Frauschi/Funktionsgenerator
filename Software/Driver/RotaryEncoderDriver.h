#ifndef ROTARYENCODERDRIVER_H_
#define ROTARYENCODERDRIVER_H_


#include <cstdint>
#include <functional>
#include <array>
#include <initializer_list>

#include "stm32l476xx.h"
#include "CircularBuffer.h"
#include "HardwareEncoder.h"



namespace Driver
{

template <typename TEncoderDevice, typename TEventLoop>
class RotaryEncoderDriver
{
public:
	typedef typename TEventLoop::Task::HandlerType 		CallbackHandler;

	//Constructor
	RotaryEncoderDriver(const TEncoderDevice& enc, const TEventLoop& el);
	//Destructor
	~RotaryEncoderDriver();

	template <typename TFunc>
	void addRotateLeftHandler(TFunc&& callback) const;

	template <typename TFunc>
	void addRotateRightHandler(TFunc&& callback) const;

	void enable(void) const;
	void disable(void) const;

private:
	const TEncoderDevice& enc_;
	const TEventLoop& el_;

	/* Callback for Encoder Device*/
	void rotateLeftComplete(void) const;
	void rotateRightComplete(void) const;

	mutable CallbackHandler callbackLeft;
	mutable CallbackHandler callbackRight;
};


//---------------------------------------------------------------------------------------
//--------------------- Implementation of Class 'EncoderDriver' ----------------------
template <typename TEncoderDevice, typename TEventLoop>
Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
RotaryEncoderDriver(const TEncoderDevice& enc, const TEventLoop& el) :
	enc_(enc),
	el_(el)
{
	/* Set OpComplete methods */
	enc_.setLeftCompleteHandler([this]() {
		rotateLeftComplete();
	});
	enc_.setRightCompleteHandler([this]() {
		rotateRightComplete();
	});
}

template <typename TEncoderDevice, typename TEventLoop>
Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
~RotaryEncoderDriver()
{
}

template <typename TEncoderDevice, typename TEventLoop>
template <typename TFunc>
void Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
addRotateLeftHandler(TFunc&& callback) const
{
	callbackLeft = std::forward<TFunc>(callback);
}

template <typename TEncoderDevice, typename TEventLoop>
template <typename TFunc>
void Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
addRotateRightHandler(TFunc&& callback) const
{
	callbackRight = std::forward<TFunc>(callback);
}

template <typename TEncoderDevice, typename TEventLoop>
void Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
enable(void) const
{
	enc_.enableEncoder();
}

template <typename TEncoderDevice, typename TEventLoop>
void Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
disable(void) const
{
	enc_.disableEncoder();
}

template <typename TEncoderDevice, typename TEventLoop>
void Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
rotateLeftComplete(void) const
{
	if (callbackLeft) {
		el_.addTaskToQueue(callbackLeft);
	}
}

template <typename TEncoderDevice, typename TEventLoop>
void Driver::RotaryEncoderDriver<TEncoderDevice, TEventLoop>::
rotateRightComplete(void) const
{
	if (callbackRight) {
		el_.addTaskToQueue(callbackRight);
	}
}


}	/*Namespace Driver*/


#endif /* ROTARYENCODERDRIVER_H_ */
