#ifndef HARDWAREI2C1_H_
#define HARDWAREI2C1_H_


#include <MiscStuff.h>
#include <cstdint>
#include <functional>
#include "stm32l476xx.h"


namespace Device
{

class HardwareI2C1
{

public:

	typedef std::function<void (MiscStuff::ErrorCode returnValue)> TOpCompleteHandler;

	// Constructor
	HardwareI2C1();

	// Destructor
	~HardwareI2C1();

	/* Start a Transmission */
	void beginTransmit(const std::uint8_t slaveAddress, const std::uint8_t* dataSrc, const std::size_t numOfBytes) const;

	/* Start a Reception */
	void beginReceive(const std::uint8_t slaveAddress, const std::uint8_t* dataDest, const std::size_t numOfBytes) const;


	// Set callback functions
	template <typename TFunc>
	void setReadCompleteHandler(TFunc&& func) const
	{
		readCompleteHandler_ = std::forward<TFunc>(func);
	}

	template <typename TFunc>
	void setWriteCompleteHandler(TFunc&& func) const
	{
		writeCompleteHandler_ = std::forward<TFunc>(func);
	}

private:

	// Storage for callback functions
	mutable TOpCompleteHandler readCompleteHandler_;
	mutable TOpCompleteHandler writeCompleteHandler_;

	// Interrupt handler
	void i2cEventHandler(void) const;
	void i2cErrorHandler(void) const;
	void dmaTxHandler(void) const;
	void dmaRxHandler(void) const;

};

} /* namespace device */

#endif /* HARDWAREI2C1_H_ */
