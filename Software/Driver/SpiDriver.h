#ifndef SPIDRIVER_H_
#define SPIDRIVER_H_


#include <MiscStuff.h>
#include <cstdint>
#include <functional>
#include <array>
#include <initializer_list>
#include <cstring>

#include "stm32l476xx.h"
#include "CircularBuffer.h"
#include "HardwareGpio.h"


namespace Driver
{

template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize = 20>
class SpiMasterBusManager
{
public:

	typedef typename TSpiDevice::DataType				DataType;
	typedef typename TSpiDevice::ClockPhase				ClockPhase;
	typedef typename TEventLoop::Task::HandlerType 		CallbackHandler;

	enum DataHandling : std::uint8_t {standard, dspCommand, dspData, ddsCommand, ddsData};

	// Constructor
	SpiMasterBusManager(const TSpiDevice& spi, const TEventLoop& el);

	//Destructor
	~SpiMasterBusManager();

	//Methods
	template <typename TFunc>
	void asyncWrite(const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin, const DataType* source,
			const std::size_t numOfBytes, const enum DataHandling, const TGpioDevice& displayCDBase,
			typename TGpioDevice::Pin const displayCDPin, TFunc&& callback) const;

	template <typename TFunc>
	void asyncRead(const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin, const std::uint8_t* dest,
			const std::size_t numOfBytes, TFunc&& callback) const;


private:

	enum Mode : std::uint8_t {Transmission, Reception};

	/* Flag to indicate if there is an ongoing task */
	volatile mutable bool busBusy_;

	/* Data structure for the transmission / reception tasks */
	struct SpiTask_t {
		enum Mode mode_;
		enum DataHandling dataHandling_;
		typename TGpioDevice::Pin slaveCsPin_;
		typename TGpioDevice::Pin displayCDPin_;
		const TGpioDevice* slaveCsBase_;
		const TGpioDevice* displayCDBase_;					//for Display Command/Data Pin
		const DataType* dataPtr_;
		std::size_t numOfBytes_;
		CallbackHandler callback_;

		SpiTask_t() :
			mode_(Mode::Transmission),
			dataHandling_(DataHandling::standard),
			slaveCsPin_(TGpioDevice::Pin::_0),
			displayCDPin_(TGpioDevice::Pin::_0),
			slaveCsBase_(nullptr),
			displayCDBase_(nullptr),
			dataPtr_(nullptr),
			numOfBytes_(0),
			callback_(nullptr)
		{
		}

		template <typename TFunc>
		SpiTask_t(Mode const mode, DataHandling const handling, typename TGpioDevice::Pin csPin,
				typename TGpioDevice::Pin cdPin, const TGpioDevice* csBase, const TGpioDevice* cdBase,
				const DataType* data, std::size_t const numOfBytes, TFunc&& callback) :
			mode_(mode),
			dataHandling_(handling),
			slaveCsPin_(csPin),
			displayCDPin_(cdPin),
			slaveCsBase_(csBase),
			displayCDBase_(cdBase),
			dataPtr_(data),
			numOfBytes_(numOfBytes),
			callback_(std::forward<TFunc>(callback))
		{
		}

		template <typename TFunc>
		SpiTask_t(Mode const mode, typename TGpioDevice::Pin csPin, const TGpioDevice* csBase,
				const DataType* data, std::size_t const numOfBytes, TFunc&& callback) :
			mode_(mode),
			dataHandling_(DataHandling::standard),
			slaveCsPin_(csPin),
			displayCDPin_(TGpioDevice::Pin::_0),
			slaveCsBase_(csBase),
			displayCDBase_(csBase),
			dataPtr_(data),
			numOfBytes_(numOfBytes),
			callback_(std::forward<TFunc>(callback))
		{
		}
	};
	mutable Util::CircularBuffer<SpiTask_t, TQueueSize> taskQueue_;

	const TSpiDevice& spi_;
	const TEventLoop& el_;

	/* Starts the hardware spi appropriately for the next task */
	void startNextTask(void) const;

	/* Callback for the Hardware SPI Device */
	void taskComplete(MiscStuff::ErrorCode returnValue) const;

};


/* Class for a SpiDriver
 * 	Each Slave in the system gets his own driver object which is managed by the BusManager.
 * 	Each driver object has its own buffer structure for Transmission and Reception of individual data.
 */
template <typename TBusManager, typename TGpioDevice, typename TDataSize>
class SpiSlaveDriver
{
public:

	typedef TDataSize							DataType;
	typedef typename TBusManager::DataHandling	DataHandling;

	//Constructors
	SpiSlaveDriver(const TBusManager& busManager, const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin);
	SpiSlaveDriver(const TBusManager& busManager, const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin,
			const TGpioDevice& displayCDBase, typename TGpioDevice::Pin const displayCDPin);

	/*Transmit operation*/
	template <typename TFunc>
	void asyncWrite(const DataType* source, const std::size_t numOfBytes, const DataHandling dataHandling, TFunc&& callback) const;

	/*Receive operation*/
	template <typename TFunc>
	void asyncRead(const DataType* dest, const std::size_t numOfBytes, TFunc&& callback) const;


private:

	const TBusManager& busManager_;
	const TGpioDevice& slaveCsBase_;
	typename TGpioDevice::Pin const slaveCsPin_;
	const TGpioDevice& displayCDBase_;
	typename TGpioDevice::Pin const displayCDPin_;

};


//---------------------------------------------------------------------------------------
// -------------------------------- Implementation --------------------------------------
template <typename TBusManager, typename TGpioDevice, typename TDataSize>
Driver::SpiSlaveDriver<TBusManager, TGpioDevice, TDataSize>::
SpiSlaveDriver(const TBusManager& busManager, const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin) :
	busManager_(busManager),
	slaveCsBase_(slaveCsBase),
	slaveCsPin_(slaveCsPin),
	displayCDBase_(slaveCsBase),
	displayCDPin_(slaveCsPin)
{
	slaveCsBase_.setPinMode(slaveCsPin_, TGpioDevice::PinMode::Output);
	slaveCsBase_.setPinOutputType(slaveCsPin_, TGpioDevice::PinOutputType::PushPull);
	slaveCsBase_.setPinOutputSpeed(slaveCsPin_, TGpioDevice::PinOutputSpeed::VeryHighSpeed);
	slaveCsBase_.setPinPullUpDown(slaveCsPin_, TGpioDevice::PinPullUpDown::None);
	slaveCsBase_.setPinStatus(slaveCsPin_, TGpioDevice::PinStatus::High);
}

template <typename TBusManager, typename TGpioDevice, typename TDataSize>
Driver::SpiSlaveDriver<TBusManager, TGpioDevice, TDataSize>::
SpiSlaveDriver(const TBusManager& busManager, const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin,
		const TGpioDevice& displayCDBase, typename TGpioDevice::Pin const displayCDPin) :
	busManager_(busManager),
	slaveCsBase_(slaveCsBase),
	slaveCsPin_(slaveCsPin),
	displayCDBase_(displayCDBase),
	displayCDPin_(displayCDPin)
{
	slaveCsBase_.setPinMode(slaveCsPin_, TGpioDevice::PinMode::Output);
	slaveCsBase_.setPinOutputType(slaveCsPin_, TGpioDevice::PinOutputType::PushPull);
	slaveCsBase_.setPinOutputSpeed(slaveCsPin_, TGpioDevice::PinOutputSpeed::VeryHighSpeed);
	slaveCsBase_.setPinPullUpDown(slaveCsPin_, TGpioDevice::PinPullUpDown::None);
	slaveCsBase_.setPinStatus(slaveCsPin_, TGpioDevice::PinStatus::High);

	displayCDBase_.setPinMode(displayCDPin_, TGpioDevice::PinMode::Output);
	displayCDBase_.setPinOutputType(displayCDPin_, TGpioDevice::PinOutputType::PushPull);
	displayCDBase_.setPinOutputSpeed(displayCDPin_, TGpioDevice::PinOutputSpeed::VeryHighSpeed);
	displayCDBase_.setPinPullUpDown(displayCDPin_, TGpioDevice::PinPullUpDown::None);
	displayCDBase_.setPinStatus(displayCDPin_, TGpioDevice::PinStatus::Low);
}


template <typename TBusManager, typename TGpioDevice, typename TDataSize>
template <typename TFunc>
void Driver::SpiSlaveDriver<TBusManager, TGpioDevice, TDataSize>::
asyncWrite(const DataType* source, const std::size_t numOfBytes, const DataHandling dataHandling, TFunc&& callback) const
{
	DataType* buff = nullptr;
	/* Create a new buffer */
	switch(dataHandling)
	{
		case DataHandling::standard:
		case DataHandling::dspCommand:
		case DataHandling::ddsCommand:
			buff = reinterpret_cast<DataType*>(malloc(numOfBytes));
			if (buff == NULL) {
				/* Error allocating new memory */
				return;
			}
			/* Copy data to be sent in the created buffer */
			std::memcpy(buff, source, numOfBytes);
			break;

		case DataHandling::dspData:
		case DataHandling::ddsData:
			buff = const_cast<DataType*>(source);
			break;

		default:
			buff = nullptr;
			break;
	}

	busManager_.asyncWrite(slaveCsBase_, slaveCsPin_, buff, numOfBytes, dataHandling, displayCDBase_, displayCDPin_, callback);
}


template <typename TBusManager, typename TGpioDevice, typename TDataSize>
template <typename TFunc>
void Driver::SpiSlaveDriver<TBusManager, TGpioDevice, TDataSize>::
asyncRead(const DataType* dest, const std::size_t numOfBytes, TFunc&& callback) const
{
}


//---------------------------------------------------------------------------------------
//--------------------- Implementation of Class 'SpiMasterManager' ----------------------
template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize>
Driver::SpiMasterBusManager<TSpiDevice, TGpioDevice, TEventLoop, TQueueSize>::
SpiMasterBusManager(const TSpiDevice& spi, const TEventLoop& el) :
	busBusy_(false),
	spi_(spi),
	el_(el)
{
	/* Set OpComplete method */
	auto opCompleteCallback = [this](MiscStuff::ErrorCode const errCode) {
		this->taskComplete(errCode);
	};

	spi_.setWriteCompleteHandler(opCompleteCallback);
	spi_.setReadCompleteHandler(opCompleteCallback);
}


template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize>
Driver::SpiMasterBusManager<TSpiDevice, TGpioDevice, TEventLoop, TQueueSize>::
~SpiMasterBusManager()
{
}


template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize>
template <typename TFunc>
void Driver::SpiMasterBusManager<TSpiDevice, TGpioDevice, TEventLoop, TQueueSize>::
asyncWrite(const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin, const DataType* source,
		const std::size_t numOfBytes, const enum DataHandling dataHandling, const TGpioDevice& displayCdBase,
		typename TGpioDevice::Pin const displayCdPin, TFunc&& callback) const
{
	/* Lock the EventLoop to prevent a race condition on the taskQueue */
	el_.lock();

	/* Add new Task to the Queue */
	taskQueue_.push(std::move(SpiTask_t(Mode::Transmission, dataHandling, slaveCsPin, displayCdPin, &slaveCsBase,
			&displayCdBase, source, numOfBytes, callback)));

	/* Unlock the EventLoop */
	el_.unlock();

	/* When there is nothing ongoing on the bus, start transmission */
	if (busBusy_ == false) {
		busBusy_ = true;

		startNextTask();
	}
}


template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize>
template <typename TFunc>
void Driver::SpiMasterBusManager<TSpiDevice, TGpioDevice, TEventLoop, TQueueSize>::
asyncRead(const TGpioDevice& slaveCsBase, typename TGpioDevice::Pin const slaveCsPin, const std::uint8_t* dest,
		const std::size_t numOfBytes, TFunc&& callback) const
{
	/* Lock the EventLoop to prevent a race condition on the taskQueue */
	el_.lock();

	/* Add new Task to the Queue */
	taskQueue_.push(std::move(SpiTask_t(Mode::Reception, slaveCsPin, &slaveCsBase, dest,
			numOfBytes, std::forward<TFunc>(callback))));

	/* Unlock the EventLoop */
	el_.unlock();

	/* When there is nothing ongoing on the bus, start transmission */
	if (busBusy_ == false) {
		busBusy_ = true;

		startNextTask();
	}
}


template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize>
void Driver::SpiMasterBusManager<TSpiDevice, TGpioDevice, TEventLoop, TQueueSize>::
startNextTask(void) const
{
	/* Lock the EventLoop to prevent a race condition on the taskQueue */
	el_.lock();

	/* Get data of next task */
	SpiTask_t const& nextTask = taskQueue_.peek();

	/* Unlock the EventLoop */
	el_.unlock();

	/* Set CS of corresponding SPI Slave to Low */
	nextTask.slaveCsBase_->setPinStatus(nextTask.slaveCsPin_, Device::HardwareGpio::PinStatus::Low);

	switch(nextTask.dataHandling_)
	{
		case standard:
			spi_.setClockPhase(ClockPhase::FirstEdge);
			break;
		case dspCommand:
			spi_.setClockPhase(ClockPhase::FirstEdge);
			if(not (nextTask.displayCDBase_ == nextTask.slaveCsBase_ && nextTask.displayCDPin_ == nextTask.slaveCsPin_))
				nextTask.displayCDBase_->setPinStatus(nextTask.displayCDPin_, Device::HardwareGpio::PinStatus::Low);
			break;
		case dspData:
			spi_.setClockPhase(ClockPhase::FirstEdge);
			if(not (nextTask.displayCDBase_ == nextTask.slaveCsBase_ && nextTask.displayCDPin_ == nextTask.slaveCsPin_))
				nextTask.displayCDBase_->setPinStatus(nextTask.displayCDPin_, Device::HardwareGpio::PinStatus::High);
			break;
		case ddsCommand:
		case ddsData:
			spi_.setClockPhase(ClockPhase::SecondEdge);
			break;
		default:
			break;
	}

	/* Start DMA for hardware spi */
	if (nextTask.mode_ == Mode::Transmission) {
		if (nextTask.dataHandling_ == DataHandling::ddsData) {
			/* In case of ddsData, we only send 4 bytes */
			spi_.beginTransmit(nextTask.dataPtr_, 4);
		}
		else {
			spi_.beginTransmit(nextTask.dataPtr_, nextTask.numOfBytes_);
		}
	}
	else if (nextTask.mode_ == Mode::Reception) {
		spi_.beginReceive(nextTask.dataPtr_, nextTask.numOfBytes_);
	}
}


/* Callback handler */
template <typename TSpiDevice, typename TGpioDevice, typename TEventLoop, std::size_t TQueueSize>
void Driver::SpiMasterBusManager<TSpiDevice, TGpioDevice, TEventLoop, TQueueSize>::
taskComplete(MiscStuff::ErrorCode returnValue) const
{
	/* Check if Task was completed successfully */
	if (returnValue == MiscStuff::ErrorCode::Success)
	{
		/* Lock the EventLoop to prevent a race condition on the taskQueue */
		el_.lock();

		/* Get just finished task and remove it from the queue */
		auto finishedTask = taskQueue_.peek();

		/* Unlock the EventLoop */
		el_.unlock();

		/* Set CS of corresponding SPI Slave to High */
		finishedTask.slaveCsBase_->setPinStatus(finishedTask.slaveCsPin_, Device::HardwareGpio::PinStatus::High);

		/* If the task was a transmission, release the memory of the sent data */
		if (finishedTask.mode_ == Mode::Transmission) {
			switch(finishedTask.dataHandling_)
			{
				case DataHandling::standard:
				case DataHandling::dspCommand:
					free(const_cast<DataType*>(finishedTask.dataPtr_));
					break;
				case DataHandling::dspData:
					break;
				case DataHandling::ddsCommand:
					free(const_cast<DataType*>(finishedTask.dataPtr_));
					break;
				case DataHandling::ddsData:
					/* Reduce numOfBytes by 4. Because we only peek() in the beginning of the method without removing
					 * it from the queue, the just finished Task we handle right now is write accessable with mutablePeek() */
					taskQueue_.mutablePeek().numOfBytes_ -= 4;
					taskQueue_.mutablePeek().dataPtr_ += 4;
					break;
				default:
					break;
			}
		}

		if ((finishedTask.dataHandling_ != DataHandling::ddsData) || (finishedTask.numOfBytes_ < 8)) {
			/* Add callback to the EventLoop queue, if callback is valid */
			if (finishedTask.callback_) {
				el_.addTaskToQueue(finishedTask.callback_);
			}

			/* Delete Task */
			el_.lock();
			taskQueue_.deleteNext();
			el_.unlock();
		}
	}

	/* In case of an error, just try again
	 * Task where the error occured is still in the queue, so just start the next task */

	/* Check if there is data to send next */
	if (taskQueue_.available()) {
		startNextTask();
	}
	else {
		busBusy_ = false;
	}
}


} /* namespace driver */

#endif /* SPIDRIVER_H_ */
