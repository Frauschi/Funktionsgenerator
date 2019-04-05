/*
 * ExternalEEPROM.h
 *
 *  Created on: 10.12.2018
 *      Author: dem34203
 */

#ifndef EXTERNALEEPROM_H_
#define EXTERNALEEPROM_H_

#include "SystemManager.h"
#include "IdManager.h"
#include <cstdint>

namespace Component
{

template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
class ExternalEEPROM
{
public:
	typedef Util::IdManager::IdType 						IdType;

	ExternalEEPROM(const TI2cSlaveDriver& i2c, const TIoPin& writeControlPin, const TEventLoop& el, const TTimer& timer);
	~ExternalEEPROM(void);

	template <typename TFunc, typename Tvalue>
	void loadValue(std::uint16_t const valueAddress, Tvalue* const pValue, TFunc&& callback) const;

	template <typename TFunc, typename Tvalue>
	void saveValue(std::uint16_t const valueAddress, const Tvalue data, TFunc&& callback) const;

	template <typename TFunc, typename TDataStruct>
	void loadMenuValues(std::uint16_t const MenuBaseAddress, TDataStruct* pDataStruct, TFunc&& callback) const;

	template <typename TFunc, typename TDataStruct>
	void saveMenuValues(std::uint16_t const MenuBaseAddress, const TDataStruct* pDataStruct, TFunc&& callback) const;

	bool isNewHardware(void) const;

private:
	/* Driver references to access system peripherals */
	const TI2cSlaveDriver& i2c_;
	const TIoPin& wcPin_;
	const TEventLoop& el_;

	mutable typename TEventLoop::Task::HandlerType loadValueCallback_;
	mutable typename TEventLoop::Task::HandlerType loadMenuValuesCallback_;

	mutable typename TEventLoop::Task::HandlerType saveValueCallback_;
	mutable typename TEventLoop::Task::HandlerType saveMenuValuesCallback_;

	/* Array for the Transmission data via I2C */
	mutable std::uint8_t outputData_[30];

	/* Pointer to a Byte Array to access the Received data via I2C */
	mutable std::uint8_t* inputData_;

	/* Variables to dedicate if a new EEPROM is used */
	mutable bool isNewHardware_;
	mutable std::uint32_t hardwareInteger_;

	/* Timer to coordinate the save functions */
	const TTimer& timer_;
	mutable IdType timerID_;
};


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
ExternalEEPROM(const TI2cSlaveDriver& i2c, const TIoPin& writeControlPin, const TEventLoop& el, const TTimer& timer) :
	i2c_(i2c),
	wcPin_(writeControlPin),
	el_(el),
	loadValueCallback_(nullptr),
	loadMenuValuesCallback_(nullptr),
	saveValueCallback_(nullptr),
	saveMenuValuesCallback_(nullptr),
	inputData_(nullptr),
	isNewHardware_(false),
	hardwareInteger_(0x00000000),
	timer_(timer),
	timerID_(0)
{
	wcPin_.setHigh();

	loadValue(0x0F00, &hardwareInteger_, [this](){
		if(hardwareInteger_ == 0xBEEF)
		{
			isNewHardware_ = false;
		}
		else
		{
			isNewHardware_ = true;
			saveValue(0x0F00, 0xBEEF, nullptr);
		}
	});
}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
~ExternalEEPROM(void)
{
}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
template <typename TFunc, typename Tvalue>
void ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
loadValue(std::uint16_t const valueAddress, Tvalue* const pValue, TFunc&& callback) const
{
	loadValueCallback_ = std::forward<TFunc>(callback);

	inputData_ = i2c_.asyncReadRegister(valueAddress, 4, nullptr, [this, pValue](){
		*pValue = static_cast<Tvalue>(inputData_[3]<<24 | inputData_[2]<<16 | inputData_[1]<<8 | inputData_[0]);

		/* Free memory of the received data */
		free(inputData_);
		inputData_ = nullptr;

		if(loadValueCallback_)
		{
			el_.addTaskToQueue(loadValueCallback_);
		}
	});
}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
template <typename TFunc, typename Tvalue>
void ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
saveValue(std::uint16_t const valueAddress, const Tvalue data, TFunc&& callback) const
{
	saveValueCallback_ = std::forward<TFunc>(callback);

	outputData_[0] = static_cast<std::uint8_t>((valueAddress>>8) & 0xFF);
	outputData_[1] = static_cast<std::uint8_t>(valueAddress & 0xFF);

	outputData_[2] = 0x00;
	outputData_[3] = 0x00;
	outputData_[4] = 0x00;
	outputData_[5] = 0x00;

	//erase EEPROM value at corresponding address and rewrite it
	i2c_.asyncWrite(outputData_, 6,
			[this](){
				wcPin_.setLow();
			},
			[this, valueAddress, data](){
				wcPin_.setHigh();

				timerID_ = timer_.asyncWait(5ms, [this, valueAddress, data](){
					outputData_[0] = static_cast<std::uint8_t>((valueAddress>>8) & 0xFF);
					outputData_[1] = static_cast<std::uint8_t>(valueAddress & 0xFF);

					outputData_[2] = static_cast<uint8_t>( data & 0x000000FF);
					outputData_[3] = static_cast<uint8_t>((data & 0x0000FF00)>>8);
					outputData_[4] = static_cast<uint8_t>((data & 0x00FF0000)>>16);
					outputData_[5] = static_cast<uint8_t>((data & 0xFF000000)>>24);

					i2c_.asyncWrite(outputData_, 6,
						[this](){
							wcPin_.setLow();
						},
						[this](){
							timerID_ = timer_.asyncWait(1ms, [this](){
								wcPin_.setHigh();
							});
							if(saveValueCallback_ != nullptr)
							{
								el_.addTaskToQueue(saveValueCallback_);
							}
						});
				});
			});
}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
template <typename TFunc, typename TDataStruct>
void ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
loadMenuValues(std::uint16_t const MenuBaseAddress, TDataStruct* pDataStruct, TFunc&& callback) const
{
	loadMenuValuesCallback_ = std::forward<TFunc>(callback);

	inputData_ = i2c_.asyncReadRegister(MenuBaseAddress, sizeof(TDataStruct), nullptr, [this, pDataStruct](){
			std::memcpy(pDataStruct, inputData_, sizeof(TDataStruct));

			/* Free memory of the received data */
			free(inputData_);
			inputData_ = nullptr;

			if(loadMenuValuesCallback_ != nullptr)
			{
				el_.addTaskToQueue(loadMenuValuesCallback_);
			}
		});
}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
template <typename TFunc, typename TDataStruct>
void ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
saveMenuValues(std::uint16_t const MenuBaseAddress, const TDataStruct* pDataStruct, TFunc&& callback) const
{
	saveMenuValuesCallback_ = std::forward<TFunc>(callback);

	outputData_[0] = static_cast<std::uint8_t>((MenuBaseAddress>>8) & 0xFF);
	outputData_[1] = static_cast<std::uint8_t>(MenuBaseAddress & 0xFF);

	for(uint32_t i = 0; i<=sizeof(TDataStruct) + 2; i++){
		outputData_[2+i] = 0x00;
	}

	i2c_.asyncWrite(outputData_, sizeof(TDataStruct) + 2,
		[this](){
			wcPin_.setLow();
		},
		[this, MenuBaseAddress, pDataStruct](){
			wcPin_.setHigh();

			timerID_ = timer_.asyncWait(6ms, [this, MenuBaseAddress, pDataStruct](){
				outputData_[0] = static_cast<std::uint8_t>((MenuBaseAddress>>8) & 0xFF);
				outputData_[1] = static_cast<std::uint8_t>(MenuBaseAddress & 0xFF);

				std::memcpy(outputData_ + 2, pDataStruct, sizeof(TDataStruct) + 2);

				i2c_.asyncWrite(outputData_, sizeof(TDataStruct) + 2,
					[this](){
						wcPin_.setLow();
					},
					[this](){
						timerID_ = timer_.asyncWait(5ms, [this](){
							wcPin_.setHigh();
						});
						if(saveMenuValuesCallback_ != nullptr)
						{
							el_.addTaskToQueue(saveMenuValuesCallback_);
						}
					});
			});
		});
}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop, typename TTimer>
bool ExternalEEPROM<TI2cSlaveDriver, TIoPin, TEventLoop, TTimer>::
isNewHardware(void) const
{
	return(isNewHardware_);
}


} //namespace Component

#endif /* EXTERNALEEPROM_H_ */
