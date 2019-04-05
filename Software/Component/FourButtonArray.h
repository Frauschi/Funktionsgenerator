
#ifndef FOURBUTTONARRAY_H_
#define FOURBUTTONARRAY_H_


#include <cstdint>
#include <functional>

#include "SystemManager.h"



namespace Component {

template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
class FourButtonArray
{
public:

	enum Button {
		_1,
		_2,
		_3,
		_4,
		ButtonCount
	};

	enum ButtonColor {
		Red,
		Green,
		Orange,
		None
	};

	struct PinMap {
		std::uint8_t Button1Press;
		std::uint8_t Button1Red;
		std::uint8_t Button1Green;
		std::uint8_t Button2Press;
		std::uint8_t Button2Red;
		std::uint8_t Button2Green;
		std::uint8_t Button3Press;
		std::uint8_t Button3Red;
		std::uint8_t Button3Green;
		std::uint8_t Button4Press;
		std::uint8_t Button4Red;
		std::uint8_t Button4Green;
	};


	/* Constructor */
	FourButtonArray(const TI2cSlaveDriver& i2c, const TIoPin& interruptPin, const TEventLoop& el);

	/* Destructor */
	~FourButtonArray();

	/* Initialize the object */
	void initialize(const PinMap& pinMap) const;

	/* Add a callback handler for a button */
	template <typename TFunc>
	void addHandlerForButton(Button const button, TFunc&& callback) const;

	/* Set the color of a button */
	void setButtonColor(Button const button, ButtonColor const color, bool ImmediateUpdate = true) const;

	/* Get the Color of a Button*/
	ButtonColor getButtonColor(Button const button);


private:

	/* Handler for the PortExpander interrupt */
	void buttonPressedInterruptHandler(void) const;

	/* Data structure to store callback and current color of the four buttons */
	struct ButtonInfo_t {
		typename TEventLoop::Task::HandlerType callbackHandler;
		ButtonColor color;
	};
	mutable std::array<ButtonInfo_t, Button::ButtonCount> buttonArray_;

	/* Array for the Transmission data via I2C */
	mutable std::uint8_t outputData_[3];

	/* Pointer to a Byte Array to access the Received data via I2C */
	mutable std::uint8_t* inputData_;

	/* Data structure to store the Pin mapping for the PortExpander
	 * Implemented as a union to access the data from outside as a struct
	 * and from inside as a byte array */
	union PinMapping {
		PinMap map;
		std::uint8_t rawBytes[sizeof(PinMap)];
	};
	mutable PinMapping pinMap_;

	/* Driver references to access system peripherals */
	const TI2cSlaveDriver& i2c_;
	const TIoPin& interruptPin_;
	const TEventLoop& el_;

	/* Arrray to save either if the button was pressed (true) or released with last interrupt*/
	mutable std::array<bool, Button::ButtonCount> buttonWasPressed_;
};


//---------------------------------------------------------------------------------------
//-------------------- Implementation of Class 'FourButtonArray' ------------------------
template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
FourButtonArray(const TI2cSlaveDriver& i2c, const TIoPin& interruptPin, const TEventLoop& el) :
	inputData_(nullptr),
	i2c_(i2c),
	interruptPin_(interruptPin),
	el_(el)
{
	using namespace System;

	/* Enable interrupt on the falling edge on the given io pin */
	interruptPin_.addHandlerForInterruptOnEdge(TIoPin::InterruptEdge::Falling,
			[this]() { this->buttonPressedInterruptHandler(); });

}


template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
~FourButtonArray()
{
}


/* Initialize the object */
template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
void FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
initialize(const PinMap& pinMap) const
{
	/* Copy given Pin Map in local storage */
	pinMap_.map = pinMap;

	/* Prefill buttonArray_ */
	for (std::size_t i = 0; i < Button::ButtonCount; i++) {
		buttonArray_[i] = {nullptr, ButtonColor::None};
	}

	/* Initialize PortExpander */
	outputData_[0] = 0x06;	/* Address of config register */
	outputData_[1] = 0x00;
	outputData_[2] = 0x00;

	/* Set the four buttonPress pins as input, all other pins are outputs */
	for (std::size_t i = 0; i < Button::ButtonCount; i++) {
		outputData_[(pinMap_.rawBytes[i*3]/8)+1] |= 0x01<<(pinMap_.rawBytes[i*3]-8*(pinMap_.rawBytes[i*3]/8));
	}

	/* Send config bytes to the portExpander */
	i2c_.asyncWrite(outputData_, 3, nullptr, nullptr);

	/* Reset outputBytes array to reflect the LED pin status (plus register address in the first byte) */
	outputData_[0] = 0x02; /* Address of output data register */
	outputData_[1] = 0x00;
	outputData_[2] = 0x00;

	/* Set output bytes appropriately so all LEDs are off */
	for (std::size_t i = 0; i < Button::ButtonCount; i++) {
		/* Red_Pin high */
		outputData_[(pinMap_.rawBytes[(i*3)+1]/8)+1] |= 0x01<<(pinMap_.rawBytes[(i*3)+1]-8*(pinMap_.rawBytes[(i*3)+1]/8));

		/* Green_Pin high */
		outputData_[(pinMap_.rawBytes[(i*3)+2]/8)+1] |= 0x01<<(pinMap_.rawBytes[(i*3)+2]-8*(pinMap_.rawBytes[(i*3)+2]/8));
	}

	/* Send outputBytes to set the LEDs */
	i2c_.asyncWrite(outputData_, 3, nullptr, nullptr);
}


/* Add a callback handler for a button */
template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
template <typename TFunc>
void FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
addHandlerForButton(Button const button, TFunc&& callback) const
{
	buttonArray_[button].callbackHandler = std::forward<TFunc>(callback);
}


/* Set the color of a button */
template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
void FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
setButtonColor(Button const button, ButtonColor const color, bool ImmediateUpdate) const
{
	/* Check if the requested color is already selected */
	if (buttonArray_[button].color != color) {
		buttonArray_[button].color = color;

		/* Update PortExpander output data register to activate the correct LED */
		switch (color) {
		case ButtonColor::Red:
			/* Red Pin low */
			outputData_[(pinMap_.rawBytes[(button*3)+1]/8)+1] &= ~(0x01<<(pinMap_.rawBytes[(button*3)+1]-8*(pinMap_.rawBytes[(button*3)+1]/8)));

			/* Green Pin high */
			outputData_[(pinMap_.rawBytes[(button*3)+2]/8)+1] |= 0x01<<(pinMap_.rawBytes[(button*3)+2]-8*(pinMap_.rawBytes[(button*3)+2]/8));

			break;

		case ButtonColor::Green:
			/* Red Pin high */
			outputData_[(pinMap_.rawBytes[(button*3)+1]/8)+1] |= 0x01<<(pinMap_.rawBytes[(button*3)+1]-8*(pinMap_.rawBytes[(button*3)+1]/8));

			/* Green Pin low */
			outputData_[(pinMap_.rawBytes[(button*3)+2]/8)+1] &= ~(0x01<<(pinMap_.rawBytes[(button*3)+2]-8*(pinMap_.rawBytes[(button*3)+2]/8)));

			break;

		case ButtonColor::Orange:
			/* Red Pin low */
			outputData_[(pinMap_.rawBytes[(button*3)+1]/8)+1] &= ~(0x01<<(pinMap_.rawBytes[(button*3)+1]-8*(pinMap_.rawBytes[(button*3)+1]/8)));

			/* Green Pin low */
			outputData_[(pinMap_.rawBytes[(button*3)+2]/8)+1] &= ~(0x01<<(pinMap_.rawBytes[(button*3)+2]-8*(pinMap_.rawBytes[(button*3)+2]/8)));

			break;

		case ButtonColor::None:
			/* Red_Pin high */
			outputData_[(pinMap_.rawBytes[(button*3)+1]/8)+1] |= 0x01<<(pinMap_.rawBytes[(button*3)+1]-8*(pinMap_.rawBytes[(button*3)+1]/8));

			/* Green_Pin high */
			outputData_[(pinMap_.rawBytes[(button*3)+2]/8)+1] |= 0x01<<(pinMap_.rawBytes[(button*3)+2]-8*(pinMap_.rawBytes[(button*3)+2]/8));

			break;
		}

		if(ImmediateUpdate) {
			/* Send updated register values to the port expander */
			i2c_.asyncWrite(outputData_, 3, nullptr, nullptr);
		}
	}
}


/* Get the Color of a Button*/
template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
typename FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::ButtonColor
FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
getButtonColor(Button const button)
{
	return(buttonArray_[button].color);
}


/* Interrupt handler; Called when a button is pressed */
template <typename TI2cSlaveDriver, typename TIoPin, typename TEventLoop>
void FourButtonArray<TI2cSlaveDriver, TIoPin, TEventLoop>::
buttonPressedInterruptHandler(void) const
{
	/* Read the PortExpander register to get the pressed button */
	uint8_t regAdress = 0x00;
	inputData_ = i2c_.asyncReadRegister(regAdress, 2, nullptr, [this]() {
		/* If there was an error allocating memory for the received data, return doing nothing */
		if (this->inputData_ == nullptr)
			return;

		/* Check which bit is low in the input register */
		for (std::size_t i = 0; i < Button::ButtonCount; i++) {

			bool buttonPressed = false;

			if( (inputData_[pinMap_.rawBytes[i*3]/8] & 0x01<<(pinMap_.rawBytes[i*3] - 8*(pinMap_.rawBytes[i*3]/8))) == 0)
			{
				buttonPressed = true;
			}

			if ( buttonPressed == true && buttonWasPressed_[i] == false)
			{
				/* Button pressed, add callback to the EventLoop*/
				if (buttonArray_[i].callbackHandler) {
					el_.addTaskToQueue(buttonArray_[i].callbackHandler);
				}
				buttonWasPressed_[i] = true;

			}
			else if (buttonPressed == false && buttonWasPressed_[i] == true)
			{

				/* Button releaed, dont add new callback but enable Interrupt again*/
				buttonWasPressed_[i] = false;
			}
		}

		/* Free memory of the received data */
		free(inputData_);
		inputData_ = nullptr;
	});
}



} /* namespace Component */

#endif /* FOURBUTTONARRAY_H_ */
