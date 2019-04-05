
#ifndef HARDWAREGPIO_H_
#define HARDWAREGPIO_H_


#include <cstdint>
#include <functional>
#include "stm32l476xx.h"
#include "HardwareCore.h"

namespace Device {


class HardwareGpio
{
public:

	/* Enum for easy access to the different pins */
	enum Pin : std::uint8_t {
		_0,
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,
		_10,
		_11,
		_12,
		_13,
		_14,
		_15,
	};

	/* Enums for the port settings */
	enum PinMode : std::uint8_t {
		Input,
		Output,
		AlternateFunction,
		Analog
	};

	enum PinOutputType : std::uint8_t {
		PushPull,
		OpenDrain
	};

	enum PinOutputSpeed : std::uint8_t {
		LowSpeed,
		MediumSpeed,
		HighSpeed,
		VeryHighSpeed
	};

	enum PinPullUpDown : std::uint8_t {
		None,
		PullUp,
		PullDown,
	};

	enum PinAlternateFunction : std::uint8_t {
		AF0,
		AF1,
		AF2,
		AF3,
		AF4,
		AF5,
		AF6,
		AF7,
		AF8,
		AF9,
		AF10,
		AF11,
		AF12,
		AF13,
		AF14,
		AF15
	};

	/* Enum for setting the status of the pin */
	enum PinStatus : std::uint8_t {
		Low,
		High
	};

	/* Constructor */
	explicit HardwareGpio(GPIO_TypeDef* base);

	/* Destructor */
	~HardwareGpio();

	/* Methods to set / change Pin settings */
	inline void setPinMode(Pin pin, PinMode mode) const;
	inline void setPinOutputType(Pin pin, PinOutputType type) const;
	inline void setPinOutputSpeed(Pin pin, PinOutputSpeed speed) const;
	inline void setPinPullUpDown(Pin pin, PinPullUpDown upDown) const;
	inline void setPinAlternateFunction(Pin pin, PinAlternateFunction af) const;

	/* Methods to set / get the Pin status */
	inline void setPinStatus(Pin pin, PinStatus status) const;
	inline PinStatus getPinStatus(Pin pin) const;

	inline Device::GpioPin getPinNumber(Pin pin) const;

private:

	/* Base address of the GPIO port */
	GPIO_TypeDef* const base_;

};


inline void HardwareGpio::
setPinMode(const Pin pin, const PinMode mode) const
{
	/* Clear corresponding bits and set new values */
	base_->MODER = (base_->MODER & ~(0x03<<(pin*2))) | mode<<(pin*2);
}

inline void HardwareGpio::
setPinOutputType(Pin pin, PinOutputType type) const
{
	/* Clear corresponding bits and set new values */
	base_->OTYPER = (base_->OTYPER & ~(0x01<<pin)) | type<<pin;
}

inline void HardwareGpio::
setPinOutputSpeed(Pin pin, PinOutputSpeed speed) const
{
	/* Clear corresponding bits and set new values */
	base_->OSPEEDR = (base_->OSPEEDR & ~(0x03<<(pin*2))) | speed<<(pin*2);
}

inline void HardwareGpio::
setPinPullUpDown(Pin pin, PinPullUpDown upDown) const
{
	/* Clear corresponding bits and set new values */
	base_->PUPDR = (base_->PUPDR & ~(0x03<<(pin*2))) | upDown<<(pin*2);
}

inline void HardwareGpio::
setPinAlternateFunction(Pin pin, PinAlternateFunction af) const
{
	/* Clear corresponding bits and set new values */
	if (pin <= Pin::_7) {
		base_->AFR[0] = (base_->AFR[0] & ~(0x0F<<(pin*4))) | af<<(pin*4);
	}
	else {
		base_->AFR[1] = (base_->AFR[1] & ~(0x0F<<((pin-8)*4))) | af<<((pin-8)*4);
	}
}

inline void HardwareGpio::
setPinStatus(Pin pin, PinStatus status) const
{
	/* Set corresponding bit in the set/reset register */
	base_->BSRR	|= 0x01<<((16 * (1 - status)) + pin);
}

inline HardwareGpio::PinStatus
HardwareGpio::
getPinStatus(Pin pin) const
{
	return static_cast<PinStatus>((base_->IDR & 0x01<<pin)>>pin);
}


inline Device::GpioPin HardwareGpio::
getPinNumber(Pin pin) const
{
	return static_cast<Device::GpioPin>((pin * 8) + ((reinterpret_cast<std::uint32_t>(base_) - AHB2PERIPH_BASE)>>10));
}


} /* namespace Device */

#endif /* HARDWAREGPIO_H_ */
