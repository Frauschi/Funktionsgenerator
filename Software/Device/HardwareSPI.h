#ifndef HARDWARESPI_H_
#define HARDWARESPI_H_

#include <MiscStuff.h>
#include <cstdint>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareGpio.h"




namespace Device
{


class HardwareSPI
{

public:

	typedef std::uint8_t	DataType;
	typedef std::function<void (MiscStuff::ErrorCode returnValue)> TOpCompleteHandler;

	enum ClockPhase : std::uint8_t {FirstEdge, SecondEdge};

	//Constructor
	explicit HardwareSPI(SPI_TypeDef* base);

	//Destructor
	~HardwareSPI();

	/* Start a Transmission */
	void beginTransmit(const DataType* dataSrc, const std::size_t numOfBytes) const;

	/* Start a Reception */
	void beginReceive(const DataType* dataDest, const std::size_t numOfBytes) const;

	/* Change the Clock phase */
	void setClockPhase(ClockPhase const phase) const;


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

	//Hardware Devices for SPI
	SPI_TypeDef* const spiBase_;

	GPIO_TypeDef* gpioBase_;

	DMA_TypeDef* dmaBase_;
	DMA_Channel_TypeDef* dmaRX_;
	DMA_Channel_TypeDef* dmaTX_;
	volatile uint8_t dmaChTX_;
	volatile uint8_t dmaChRX_;
	mutable volatile ClockPhase currentClkPhase_;

	// Storage for callback functions
	mutable TOpCompleteHandler readCompleteHandler_;
	mutable TOpCompleteHandler writeCompleteHandler_;

	//Interrupt Handler
	void spiHandler(void) const;
	void dmaTxHandler(void) const;
	void dmaRxHandler(void) const;
};



} /* Namespace Device*/

#endif /* HARDWARESPI_H_ */
