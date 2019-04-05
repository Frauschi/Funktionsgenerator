
#ifndef IOPINDRIVER_H_
#define IOPINDRIVER_H_


#include <cstdint>
#include <functional>
#include <array>


namespace Driver {


template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
class IoPinDriver
{
public:

	enum PinType {
		Input,
		Output_PushPull,
		Output_OpenDrain
	};

	typedef typename TGpioDevice::PinPullUpDown PullUpDown;
	typedef Device::Edge InterruptEdge;
	typedef Device::GpioIntId InterruptId;
	typedef typename TEventLoop::Task::HandlerType	CallbackHandler;

	// Constructor
	IoPinDriver(const TGpioDevice& gpio, typename TGpioDevice::Pin const pin, PinType const type, InterruptId const intId, const TEventLoop& el);

	// Destructor
	~IoPinDriver();

	/* Configure Pull up / Pull down */
	void setPullUpPullDown(PullUpDown const status) const;

	/* Set Pin status */
	void setHigh(void) const
	{
		gpio_.setPinStatus(pin_, Device::HardwareGpio::PinStatus::High);
	}

	void setLow(void) const
	{
		gpio_.setPinStatus(pin_, Device::HardwareGpio::PinStatus::Low);
	}

	/* Get Pin status */
	bool isHigh(void) const
	{
		if (gpio_.getPinStatus(pin_) == Device::HardwareGpio::PinStatus::High)
			return true;
		else
			return false;
	}

	bool isLow(void) const
	{
		if (gpio_.getPinStatus(pin_) == Device::HardwareGpio::PinStatus::Low)
			return true;
		else
			return false;
	}

	/* Toggle pin status */
	void toggle(void) const;

	/* Add interrupt on specified edge */
	template <typename TFunc>
	void addHandlerForInterruptOnEdge(InterruptEdge const edge, TFunc&& callback) const;

	void activateEdgeInterrupt(InterruptId const id) const;
	void deactivateEdgeInterrupt(InterruptId const id) const;

private:

	void edgeInterruptHandler(void) const;

	mutable CallbackHandler edgeInterruptCallback_;

	const TGpioDevice& gpio_;
	typename TGpioDevice::Pin const pin_;
	const TEventLoop& el_;
};


//---------------------------------------------------------------------------------------
//---------------------- Implementation of Class 'IoPinDriver' --------------------------
template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
IoPinDriver(const TGpioDevice& gpio, typename TGpioDevice::Pin const pin, PinType const type, InterruptId const intId, const TEventLoop& el) :
	gpio_(gpio),
	pin_(pin),
	el_(el)
{
	using namespace Device;

	/* Configure Pin */
	switch (type) {
	case Input:
		gpio_.setPinMode(pin_, HardwareGpio::PinMode::Input);
		break;
	case Output_PushPull:
		gpio_.setPinMode(pin_, HardwareGpio::PinMode::Output);
		gpio_.setPinOutputType(pin_, HardwareGpio::PinOutputType::PushPull);
		gpio_.setPinOutputSpeed(pin_, HardwareGpio::PinOutputSpeed::VeryHighSpeed);
		break;
	case Output_OpenDrain:
		gpio_.setPinMode(pin_, HardwareGpio::PinMode::Output);
		gpio_.setPinOutputType(pin_, HardwareGpio::PinOutputType::OpenDrain);
		gpio_.setPinOutputSpeed(pin_, HardwareGpio::PinOutputSpeed::VeryHighSpeed);
		break;
	default:
		break;
	}

	/* Set default value for callback to an invalid state */
	edgeInterruptCallback_ = nullptr;

	/* Set local callback in the Hardware Interrupt Manager */
	TInterruptMgr::reference().addHandlerForGpioInterrupt(intId, gpio_.getPinNumber(pin_),
			[this]() { this->edgeInterruptHandler(); });
}


template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
~IoPinDriver()
{
}


template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
void IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
setPullUpPullDown(PullUpDown status) const
{
	gpio_.setPullUpDown(pin_, status);
}


template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
void IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
toggle(void) const
{
	if (gpio_.getPinStatus(pin_) == Device::HardwareGpio::PinStatus::High)
		gpio_.setPinStatus(pin_, Device::HardwareGpio::PinStatus::Low);
	else
		gpio_.setPinStatus(pin_, Device::HardwareGpio::PinStatus::High);
}


template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
template <typename TFunc>
void IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
addHandlerForInterruptOnEdge(InterruptEdge const edge, TFunc&& callback) const
{
	/* Set callback for specified edge in the Hardware Interrupt Manager */
	TInterruptMgr::reference().enableGpioIntForEdge(gpio_.getPinNumber(pin_), edge);

	/* Store callback */
	edgeInterruptCallback_ = std::forward<TFunc>(callback);
}

template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
void IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
activateEdgeInterrupt(InterruptId const id) const
{
	TInterruptMgr::reference().activateEdgeInterrupt(id);
}

template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
void IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
deactivateEdgeInterrupt(InterruptId const id) const
{
	TInterruptMgr::reference().deactivateEdgeInterrupt(id);
}

template <typename TGpioDevice, typename TInterruptMgr, typename TEventLoop>
void IoPinDriver<TGpioDevice, TInterruptMgr, TEventLoop>::
edgeInterruptHandler(void) const
{
	/* Add callback to the EventLoop */
	if (edgeInterruptCallback_) {
		el_.addTaskToQueue(edgeInterruptCallback_);
	}
}



} /* namespace Driver */

#endif /* IOPINDRIVER_H_ */
