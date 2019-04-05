#ifndef I2CMASTERDRIVER_H_
#define I2CMASTERDRIVER_H_


#include <MiscStuff.h>
#include <cstdint>
#include <functional>
#include <array>
#include <cstring>
#include <memory>


#include "CircularBuffer.h"


namespace Driver
{

template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize = 20>
class I2cMasterBusManager
{
public:

	typedef typename TEventLoop::Task::HandlerType CallbackHandler;

	// Constructor
	I2cMasterBusManager(const TI2cDevice& i2c, const TEventLoop& el);

	// Destructor
	~I2cMasterBusManager();

	/* Methods to send / receive data via the I2C bus */
	template <typename PreCallType, typename PostCallType>
	void asyncWrite(const std::uint8_t slaveAddr, const std::uint8_t* source, const std::size_t numOfBytes,
			PreCallType&& preCall, PostCallType&& postCall) const;

	template <typename PreCallType, typename PostCallType>
	void asyncRead(const std::uint8_t slaveAddr, const std::uint8_t* dest, const std::size_t numOfBytes,
			PreCallType&& preCall, PostCallType&& postCall) const;


private:

	enum Mode : std::uint8_t {Transmission, Reception};

	/* Flag to indicate if there is an ongoing task */
	volatile mutable bool busBusy_;

	/* Data structure for the transmission / reception tasks */
	struct I2cTask_t {
		enum Mode 			mode_;
		std::uint8_t 		slaveAddr_;
		const std::uint8_t*	dataPtr_;
		std::size_t 		numOfBytes_;
		CallbackHandler 	preCall_;
		CallbackHandler 	postCall_;

		// Default constructor
		I2cTask_t() :
			mode_(Transmission),
			slaveAddr_(0),
			dataPtr_(nullptr),
			numOfBytes_(0),
			preCall_(nullptr),
			postCall_(nullptr)
		{
		}

		template <typename PreCallType, typename PostCallType>
		I2cTask_t(Mode const mode, std::uint8_t const slaveAddr, std::uint8_t const* data,
				std::size_t const numOfBytes, PreCallType&& preCall, PostCallType&& postCall) :
			mode_(mode),
			slaveAddr_(slaveAddr),
			dataPtr_(data),
			numOfBytes_(numOfBytes),
			preCall_(std::forward<PreCallType>(preCall)),
			postCall_(std::forward<PostCallType>(postCall))
		{
		}

	};
	mutable Util::CircularBuffer<I2cTask_t, TQueueSize> taskQueue_;

	const TI2cDevice& i2c_;
	const TEventLoop& el_;


	/* Starts the hardware device appropriately for the next task */
	void startNextTask(void) const;

	/* Callback for the hardware device */
	void taskComplete(MiscStuff::ErrorCode const returnValue) const;
};


/* Class for a I2cDriver
 * 	Each Slave in the system gets his own driver object which is managed by the BusManager.
 * 	Each driver object has its own buffer structure for Transmission and Reception of individual data.
 */
template <typename TBusManager>
class I2cSlaveDriver
{
public:

	// Constructors
	I2cSlaveDriver(const TBusManager& busManager, std::uint8_t const slaveAddress);

	/* Transmit operations */
	template <typename PreCallType, typename PostCallType>
	void asyncWriteRegister(const std::uint8_t regAddr, const std::uint8_t data, PreCallType&& preCall,
			PostCallType&& postCall) const;

	template <typename PreCallType, typename PostCallType>
	void asyncWrite(const std::uint8_t* source, const std::size_t numOfBytes, PreCallType&& preCall,
			PostCallType&& postCall) const;

	/* Receive operations */
	template <typename RegisterAddressType, typename PreCallType, typename PostCallType>
	std::uint8_t* asyncReadRegister(const RegisterAddressType regAddr, const std::size_t numOfBytes,
			PreCallType&& preCall, PostCallType&& postCall) const;


private:

	const TBusManager& busManager_;
	const std::uint8_t slaveAddress_;
};



//---------------------------------------------------------------------------------------
//--------------------- Implementation of Class 'I2cSlaveDriver' ------------------------
template <typename TBusManager>
Driver::I2cSlaveDriver<TBusManager>::
I2cSlaveDriver(const TBusManager& busManager, std::uint8_t const slaveAddress) :
	busManager_(busManager),
	slaveAddress_(slaveAddress)
{
}


template <typename TBusManager>
template <typename PreCallType, typename PostCallType>
void Driver::I2cSlaveDriver<TBusManager>::
asyncWriteRegister(const std::uint8_t regAddr, const std::uint8_t data, PreCallType&& preCall, PostCallType&& postCall) const
{
	/* Create a new buffer for storing the data to be sent */
	std::uint8_t* buffer = reinterpret_cast<std::uint8_t*>(malloc(2));
	if (buffer == NULL) {
		/* Error allocating new memory */
		return;
	}

	/* Fill buffer */
	buffer[0] = regAddr;
	buffer[1] = data;

	/* Call method of MasterBusManager */
	busManager_.asyncWrite(slaveAddress_, buffer, 2, preCall, postCall);
}


template <typename TBusManager>
template <typename PreCallType, typename PostCallType>
void Driver::I2cSlaveDriver<TBusManager>::
asyncWrite(const std::uint8_t* source, const std::size_t numOfBytes, PreCallType&& preCall, PostCallType&& postCall) const
{
	/* Create a new buffer */
	std::uint8_t* buffer = reinterpret_cast<std::uint8_t*>(malloc(numOfBytes));
	if (buffer == NULL) {
		/* Error allocating new memory */
		return;
	}

	/* Copy data to be sent in the created buffer */
	std::memcpy(buffer, source, numOfBytes);

	/* Call method of busManager */
	busManager_.asyncWrite(slaveAddress_, buffer, numOfBytes, preCall, postCall);
}


template <typename TBusManager>
template <typename RegisterAddressType, typename PreCallType, typename PostCallType>
std::uint8_t* Driver::I2cSlaveDriver<TBusManager>::
asyncReadRegister(const RegisterAddressType regAddr, const std::size_t numOfBytes, PreCallType&& preCall, PostCallType&& postCall) const
{
	/* To read the value of a specific register of a I2C slave, you have to first send the address
	 * of the register to the slave. Then you start a new communication to read the data.
	 * Therefore, we add two Tasks to the MasterBusManager. One to just write the register address
	 * to the slave and then one to read out the answer, which is the value of that register!
	 */

	/* Create two new buffers:
	 * 	A buffer for the register address which have to be transmitted first (1 or 2 bytes long)
	 * 	A buffer with a length of 'numOfBytes' to store the data coming from the slave
	 */
	std::uint8_t* transBuf = reinterpret_cast<std::uint8_t*>(malloc(sizeof(RegisterAddressType)));
	std::uint8_t* recvBuf = new std::uint8_t[numOfBytes];
	if (transBuf == NULL || recvBuf == NULL) {
		/* Error allocating new memory */
		return nullptr;
	}

	/* Copy the address of the register to read from in the buffer */
	if (sizeof(RegisterAddressType) == 2) {
		transBuf[0] = static_cast<std::uint8_t>(regAddr>>8);
		transBuf[1] = static_cast<std::uint8_t>(regAddr & 0xFF);
	}
	else {
		transBuf[0] = regAddr;
	}
	/* Add the task for the register address transmission to the slave.
	 * We dont want to have a callback here, therefore we pass nullptr as a callback.
	 */
	busManager_.asyncWrite(slaveAddress_, transBuf, sizeof(RegisterAddressType), preCall, nullptr);

	/* Add the task for the reception of the register value.
	 * Callback is the passed callback method parameter.
	 */
	busManager_.asyncRead(slaveAddress_, recvBuf, numOfBytes, nullptr, postCall);

	/* Return the pointer to the receive buffer */
	return recvBuf;
}



//---------------------------------------------------------------------------------------
//--------------------- Implementation of Class 'I2cBusManager' -------------------------
template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize>
Driver::I2cMasterBusManager<TI2cDevice, TEventLoop, TQueueSize>::
I2cMasterBusManager(const TI2cDevice& i2c, const TEventLoop& el) :
	busBusy_(false),
	i2c_(i2c),
	el_(el)
{
	/* Set I2C OpComplete callback methods */
	auto callbackWrapper = [this](MiscStuff::ErrorCode const errCode) {
		this->taskComplete(errCode);
	};

	i2c_.setWriteCompleteHandler(callbackWrapper);
	i2c_.setReadCompleteHandler(callbackWrapper);
}


template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize>
Driver::I2cMasterBusManager<TI2cDevice, TEventLoop, TQueueSize>::
~I2cMasterBusManager()
{
}


template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize>
template <typename PreCallType, typename PostCallType>
void Driver::I2cMasterBusManager<TI2cDevice, TEventLoop, TQueueSize>::
asyncWrite(const std::uint8_t slaveAddr, const std::uint8_t* source, const std::size_t numOfBytes, PreCallType&& preCall, PostCallType&& postCall) const
{
	/* Lock the EventLoop to prevent a race condition on the taskQueue */
	el_.lock();

	/* Add new Task to the Queue */
	taskQueue_.push(std::move(I2cTask_t(Mode::Transmission, slaveAddr, source, numOfBytes, preCall, postCall)));

	/* Unlock the EventLoop */
	el_.unlock();

	/* When there is nothing ongoing on the bus, start transmission */
	if (busBusy_ == false) {
		busBusy_ = true;

		startNextTask();
	}
}


template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize>
template <typename PreCallType, typename PostCallType>
void Driver::I2cMasterBusManager<TI2cDevice, TEventLoop, TQueueSize>::
asyncRead(const std::uint8_t slaveAddr, const std::uint8_t* dest, const std::size_t numOfBytes, PreCallType&& preCall, PostCallType&& postCall) const
{
	/* Lock the EventLoop to prevent a race condition on the taskQueue */
	el_.lock();

	/* Add new Task to the Queue */
	taskQueue_.push(std::move(I2cTask_t(Mode::Reception, slaveAddr, dest, numOfBytes, preCall, postCall)));

	/* Unlock the EventLoop */
	el_.unlock();

	/* When there is nothing ongoing on the bus, start transmission */
	if (busBusy_ == false) {
		busBusy_ = true;

		startNextTask();
	}
}


template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize>
void Driver::I2cMasterBusManager<TI2cDevice, TEventLoop, TQueueSize>::
startNextTask(void) const
{
	/* Lock the EventLoop to prevent a race condition on the taskQueue */
	el_.lock();

	/* Get data of next task */
	I2cTask_t const& nextTask = taskQueue_.peek();

	/* Unlock the EventLoop */
	el_.unlock();

	/* Call preCall method */
	if (nextTask.preCall_) {
		nextTask.preCall_();
	}

	/* Start hardware device */
	if (nextTask.mode_ == Mode::Transmission) {
		i2c_.beginTransmit(nextTask.slaveAddr_, nextTask.dataPtr_, nextTask.numOfBytes_);
	}
	else if (nextTask.mode_ == Mode::Reception) {
		i2c_.beginReceive(nextTask.slaveAddr_, nextTask.dataPtr_, nextTask.numOfBytes_);
	}
}


/* Callback handler */
template <typename TI2cDevice, typename TEventLoop, std::size_t TQueueSize>
void Driver::I2cMasterBusManager<TI2cDevice, TEventLoop, TQueueSize>::
taskComplete(MiscStuff::ErrorCode returnValue) const
{
	/* Check if Task was completed successfully */
	if (returnValue == MiscStuff::ErrorCode::Success) {

		/* Lock the EventLoop to prevent a race condition on the taskQueue */
		el_.lock();

		/* Get a reference of the just finished task to prevent a copy */
		I2cTask_t const& finishedTask = taskQueue_.peek();

		/* Unlock the EventLoop */
		el_.unlock();

		/* Add callback to the EventLoop queue */
		if (finishedTask.postCall_) {
			el_.addTaskToQueue(finishedTask.postCall_);
		}

		/* If the task was a transmission, release the memory of the sent data */
		if (finishedTask.mode_ == Mode::Transmission) {
			free(const_cast<std::uint8_t*>(finishedTask.dataPtr_));
		}

		/* remove just finished task from the queue */
		el_.lock();
		taskQueue_.deleteNext();
		el_.unlock();
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

#endif /* I2CMASTERDRIVER_H_ */
