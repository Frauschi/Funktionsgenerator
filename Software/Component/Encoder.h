
#ifndef ENCODER_H_
#define ENCODER_H_

#include <cstdint>
#include <functional>
#include <math.h>
#include "MiscStuff.h"

namespace Component {


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
class Encoder
{
public:
	typedef typename TEventLoop::Task::HandlerType 		CallbackHandler;
	typedef typename MiscStuff::EncoderCommaPosition 	Comma;
	typedef typename MiscStuff::EncoderDigit			Digits;
	typedef typename MiscStuff::EncoderValueSign		Sign;
	typedef typename MiscStuff::EncoderMode				Mode;

	/*Constructor*/
	Encoder(const TRotaryEncoder& encoder, const TIoPin& interruptPin, const TEventLoop& el);

	/*Destructor*/
	~Encoder();

	/* Add callback handler for Encoder functions */
	template <typename TFunc>
	void addPressedHandler(TFunc&& callback) const;

	template <typename TFunc>
	void addRotateLeftHandler(TFunc&& callback) const;

	template <typename TFunc>
	void addRotateRightHandler(TFunc&& callback) const;

	/* Enable and disable encoder */
	void enable(bool isSignedValue) const;
	void disable(void) const;

	void resetSign(void) const;
	void calculateDigitsFromValue(uint32_t value, int8_t const factor, Sign const sign = Sign::_positive) const;
	void calculateDigitsFromValue(int32_t value, int8_t const factor) const;

	void toggleMode(void) const;
	void toggleSign(void) const;
	void leftTurn(void) const;
	void rightTurn(void) const;
	void multiplyByTen(void) const;
	void divideByTen(void) const;

	template <typename Tvalue>
	Tvalue calculateValueFromDigits(int8_t unitFactor) const;

	uint8_t getDigit(Digits const position) const;
	Digits getPosition(void) const;
	Comma getComma(void) const;
	Sign getSign(void) const;
	int8_t getFactor(void) const;
	Mode getMode(void) const;

private:
	/* Handler for the Encoder Pressed interrupt */
	void pressedInterruptHandler(void) const;

	/* Callback for Encoder Pressed */
	mutable CallbackHandler encoderPressedCallback_;

	const TRotaryEncoder& rotaryEncoder_;
	const TIoPin& interruptPin_;
	const TEventLoop& el_;

	mutable Mode mode_;
	mutable uint8_t digits_[Digits::AmountOfDigits];
	mutable Digits currentDigitPosition_;
	mutable Comma currentCommaPosition_;
	mutable Sign currentSign_;
	mutable int8_t currentDigitsFactor_;
	mutable int8_t currentDigitsMinimumFactor_;
	mutable bool isSignedValue_;

	void decreaseDigit(Digits position) const;
	void increaseDigit(Digits position) const;

	void shiftCommaLeft(void) const;
	void shiftCommaRight(void) const;
};


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
Encoder(const TRotaryEncoder& encoder, const TIoPin& interruptPin, const TEventLoop& el) :
	encoderPressedCallback_(nullptr),
	rotaryEncoder_(encoder),
	interruptPin_(interruptPin),
	el_(el),
	mode_(Mode::_changePosition),
	currentDigitPosition_(Digits::_firstDigit),
	currentCommaPosition_(Comma::_noComma),
	currentSign_(Sign::_positive),
	currentDigitsFactor_(0),
	currentDigitsMinimumFactor_(0),
	isSignedValue_(false)
{
	using namespace System;

	/* Enable interrupt on the falling edge on the given io pin */
	interruptPin_.addHandlerForInterruptOnEdge(TIoPin::InterruptEdge::Falling, [this]() {
		pressedInterruptHandler();
	});

	for(size_t i = 0; i<Digits::AmountOfDigits; i++)
	{
		digits_[i] = 0;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
~Encoder()
{
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
pressedInterruptHandler(void) const
{
	if (encoderPressedCallback_) {
		el_.addTaskToQueue(encoderPressedCallback_);
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
template <typename TFunc>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
addPressedHandler(TFunc&& callback) const
{
	encoderPressedCallback_ = std::forward<TFunc>(callback);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
template <typename TFunc>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
addRotateLeftHandler(TFunc&& callback) const
{
	rotaryEncoder_.addRotateLeftHandler(callback);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
template <typename TFunc>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
addRotateRightHandler(TFunc&& callback) const
{
	rotaryEncoder_.addRotateRightHandler(callback);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
enable(bool isSignedValue) const
{
	isSignedValue_ = isSignedValue;
	rotaryEncoder_.enable();
	interruptPin_.activateEdgeInterrupt(TIoPin::InterruptId::encoderInt);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
disable(void) const
{
	rotaryEncoder_.disable();
	interruptPin_.deactivateEdgeInterrupt(TIoPin::InterruptId::encoderInt);
	isSignedValue_ = false;
	resetSign();
}

template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
resetSign(void) const
{
	currentSign_ = Sign::_positive;
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
calculateDigitsFromValue(uint32_t const value, int8_t const factor, Sign const sign) const
{
	currentDigitsMinimumFactor_ = factor;

	uint32_t matchedValue = value;
	int8_t matchedFactor = factor;
	uint8_t digitsDeci[2] = {0,0};

	if(factor != 0)
	{
		matchedValue = (uint32_t) value*pow(10, factor);

		if(matchedValue == 0)
		{
			matchedValue = value;
		}
		else if(matchedValue > 0 && matchedValue < 1e3)
		{
			digitsDeci[0] = (value%1000) / 100;
			digitsDeci[1] = (value%100) / 10;

			matchedFactor += 3;
		}
	}
	else
	{
		if(matchedValue >= 1e6)
		{
			digitsDeci[0] = (matchedValue%1000000) / 100000;
			digitsDeci[1] = (matchedValue%100000) / 10000;

			matchedFactor += 6;
			matchedValue /= 1e6;
		}
		else if(matchedValue >= 1e3)
		{
			digitsDeci[0] = (matchedValue%1000) / 100;
			digitsDeci[1] = (matchedValue%100) / 10;

			matchedFactor += 3;
			matchedValue /= 1e3;
		}
	}

	digits_[Digits::_firstDigit] 	= matchedValue/100;
	digits_[Digits::_secondDigit] 	= (matchedValue%100)/10;
	digits_[Digits::_thirdDigit] 	= (matchedValue%10);
	currentDigitsFactor_ = matchedFactor;

	if(matchedFactor == factor)
	{
		currentCommaPosition_ = Comma::_noComma;
	}
	else
	{
		if(digits_[Digits::_firstDigit] == 0 && digits_[Digits::_secondDigit] != 0)
		{
			digits_[Digits::_firstDigit] = digits_[Digits::_secondDigit];
			digits_[Digits::_secondDigit] = digits_[Digits::_thirdDigit];
			digits_[Digits::_thirdDigit] = digitsDeci[0];

			currentCommaPosition_ = Comma::_afterSecondDigit;
		}
		else if(digits_[Digits::_firstDigit] == 0 && digits_[Digits::_secondDigit] == 0)
		{
			digits_[Digits::_firstDigit] = digits_[Digits::_thirdDigit];
			digits_[Digits::_secondDigit] = digitsDeci[0];
			digits_[Digits::_thirdDigit] = digitsDeci[1];

			currentCommaPosition_ = Comma::_afterFirstDigit;
		}
		else
		{
			currentCommaPosition_ = Comma::_noComma;
		}
	}

	currentSign_ = sign;
	currentDigitPosition_ = Digits::_thirdDigit;
	mode_ = Mode::_changePosition;
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
calculateDigitsFromValue(int32_t const value, int8_t const factor) const
{
	uint32_t absoluteValue = 0;
	Sign sign = Sign::_positive;

	if(value<0)
	{
		sign = Sign::_negative;
	}

	absoluteValue = abs(value);

	calculateDigitsFromValue(absoluteValue, factor, sign);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
toggleMode(void) const
{
	if(mode_ == Mode::_changePosition)
	{
		mode_ = Mode::_changeDigit;
	}
	else if(mode_ == Mode::_changeDigit)
	{
		mode_ = Mode::_changePosition;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
toggleSign(void) const
{
	if(currentSign_ == Sign::_negative)
	{
		currentSign_ = Sign::_positive;
	}
	else
	{
		currentSign_ = Sign::_negative;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
leftTurn(void) const
{
	if(mode_ == Mode::_changePosition)
	{
		switch(currentDigitPosition_)
		{
		case Digits::_firstDigit: currentDigitPosition_ = Digits::_thirdDigit; break;
		case Digits::_secondDigit: currentDigitPosition_ = Digits::_firstDigit; break;
		case Digits::_thirdDigit: currentDigitPosition_ = Digits::_secondDigit; break;
		default: break;
		}
	}
	else if(mode_ == Mode::_changeDigit)
	{
		if(currentSign_ == Sign::_negative)
		{
			increaseDigit(currentDigitPosition_);
		}
		else
		{
			decreaseDigit(currentDigitPosition_);
		}
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
rightTurn(void) const
{
	if(mode_ == Mode::_changePosition)
	{
		switch(currentDigitPosition_)
		{
		case Digits::_firstDigit: currentDigitPosition_ = Digits::_secondDigit; break;
		case Digits::_secondDigit: currentDigitPosition_ = Digits::_thirdDigit; break;
		case Digits::_thirdDigit: currentDigitPosition_ = Digits::_firstDigit; break;
		default: break;
		}
	}
	else if(mode_ == Mode::_changeDigit)
	{
		if(currentSign_ == Sign::_negative)
		{
			decreaseDigit(currentDigitPosition_);
		}
		else
		{
			increaseDigit(currentDigitPosition_);
		}
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
multiplyByTen(void) const
{
	if(currentDigitsFactor_ > currentDigitsMinimumFactor_)
	{
		if(currentCommaPosition_ == Comma::_noComma)
		{
			currentDigitsFactor_ += 3;
		}
		shiftCommaRight();
	}
	else if(currentDigitsFactor_ == currentDigitsMinimumFactor_)
	{
		currentCommaPosition_ = Comma::_noComma;
		if(digits_[Digits::_firstDigit] == 0)
		{
			digits_[Digits::_firstDigit] = digits_[Digits::_secondDigit];
			digits_[Digits::_secondDigit] = digits_[Digits::_thirdDigit];
			digits_[Digits::_thirdDigit] = 0;
		}
		else
		{
			currentDigitsFactor_ += 3;
			shiftCommaRight();
		}
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
divideByTen(void) const
{
	if(currentDigitsFactor_ > currentDigitsMinimumFactor_)
	{
		if(currentCommaPosition_ == Comma::_afterFirstDigit)
		{
			currentDigitsFactor_ -= 3;
		}
		shiftCommaLeft();
	}
	else if(currentDigitsFactor_ == currentDigitsMinimumFactor_)
	{
		currentCommaPosition_ = Comma::_noComma;
		digits_[Digits::_thirdDigit] = digits_[Digits::_secondDigit];
		digits_[Digits::_secondDigit] = digits_[Digits::_firstDigit];
		digits_[Digits::_firstDigit] = 0;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
template <typename Tvalue>
Tvalue Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
calculateValueFromDigits(int8_t unitFactor) const
{
	Tvalue newValue = 0;
	int8_t matchedFactor = currentDigitsFactor_ - unitFactor;

	if(currentCommaPosition_ == Comma::_noComma)
	{
		newValue += digits_[Digits::_firstDigit] 	* pow(10, matchedFactor+2);
		newValue += digits_[Digits::_secondDigit] 	* pow(10, matchedFactor+1);
		newValue += digits_[Digits::_thirdDigit] 	* pow(10, matchedFactor);
	}
	else if(currentCommaPosition_ == Comma::_afterSecondDigit)
	{
		newValue += digits_[Digits::_firstDigit] 	* pow(10, matchedFactor+1);
		newValue += digits_[Digits::_secondDigit] 	* pow(10, matchedFactor);
		newValue += digits_[Digits::_thirdDigit] 	* pow(10, matchedFactor-1);
	}
	else if(currentCommaPosition_ == Comma::_afterFirstDigit)
	{
		newValue += digits_[Digits::_firstDigit] 	* pow(10, matchedFactor);
		newValue += digits_[Digits::_secondDigit] 	* pow(10, matchedFactor-1);
		newValue += digits_[Digits::_thirdDigit] 	* pow(10, matchedFactor-2);
	}

	newValue *= currentSign_;

	return(newValue);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
uint8_t Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
getDigit(Digits const position) const
{
	return(digits_[position]);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
MiscStuff::EncoderDigit Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
getPosition(void) const
{
	return(currentDigitPosition_);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
MiscStuff::EncoderCommaPosition Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
getComma(void) const
{
	return(currentCommaPosition_);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
MiscStuff::EncoderValueSign Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
getSign(void) const
{
	return(currentSign_);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
int8_t Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
getFactor(void) const
{
	return(currentDigitsFactor_);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
MiscStuff::EncoderMode Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
getMode(void) const
{
	return(mode_);
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
decreaseDigit(Digits const position) const
{
	if(digits_[position] == 1)
	{
		if(position == Digits::_firstDigit)
		{
			if(currentDigitsFactor_ == currentDigitsMinimumFactor_)
			{
				digits_[Digits::_firstDigit] = 0;
				digits_[Digits::_secondDigit] = 9;
				if(currentDigitPosition_ == Digits::_firstDigit)
				{
					currentDigitPosition_ = Digits::_secondDigit;
				}
			}
			else if(currentDigitsFactor_ > currentDigitsMinimumFactor_)
			{
				if(currentCommaPosition_ == Comma::_afterFirstDigit)
				{
					currentDigitsFactor_ -= 3;
				}
				shiftCommaLeft();
				digits_[Digits::_firstDigit] = 9;
				digits_[Digits::_secondDigit] = digits_[Digits::_thirdDigit];
				digits_[Digits::_thirdDigit] = 0;
			}
		}
		else if(position == Digits::_secondDigit && digits_[Digits::_firstDigit] == 0 && currentDigitsFactor_ == currentDigitsMinimumFactor_)
		{
			digits_[Digits::_secondDigit] = 0;
			digits_[Digits::_thirdDigit] = 9;
			currentDigitPosition_ = Digits::_thirdDigit;
		}
		else if(position == Digits::_thirdDigit && digits_[Digits::_firstDigit] == 0 && digits_[Digits::_secondDigit] == 0 && currentDigitsFactor_ == currentDigitsMinimumFactor_)
		{
			digits_[Digits::_thirdDigit] = 0;
			currentSign_ = Sign::_positive;
		}
		else
		{
			digits_[position] --;
		}
	}
	else if(digits_[position] == 0)
	{
		if(digits_[Digits::_firstDigit] == 0 && digits_[Digits::_secondDigit] == 0 && digits_[Digits::_thirdDigit] == 0)
		{
			if(isSignedValue_ == true)
			{
				toggleSign();
				digits_[Digits::_thirdDigit] = 1;
			}
			else
			{
				//do nothing
			}
		}
		else if(position == Digits::_secondDigit)
		{
			digits_[position] = 9;
			decreaseDigit(Digits::_firstDigit);
		}
		else if(position == Digits::_thirdDigit)
		{
			digits_[position] = 9;
			decreaseDigit(Digits::_secondDigit);
		}
	}
	else
	{
		digits_[position] --;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
increaseDigit(Digits position) const
{
	if(digits_[position] == 9)
	{
		if(position == Digits::_firstDigit)
		{
			if(currentCommaPosition_ == Comma::_noComma)
			{
				currentDigitsFactor_ += 3;
			}
			shiftCommaRight();
			digits_[Digits::_thirdDigit] = digits_[Digits::_secondDigit];
			digits_[Digits::_secondDigit] = 0;
			digits_[Digits::_firstDigit] = 1;
		}
		else if(position == Digits::_secondDigit)
		{
			increaseDigit(Digits::_firstDigit);
			digits_[position] = 0;
		}
		else if(position == Digits::_thirdDigit)
		{
			increaseDigit(Digits::_secondDigit);
			digits_[position] = 0;
		}
	}
	else
	{
		digits_[position] ++;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
shiftCommaLeft(void) const
{
	if(currentCommaPosition_ == Comma::_afterFirstDigit)
	{
		currentCommaPosition_ = Comma::_noComma;
	}
	else if(currentCommaPosition_ == Comma::_afterSecondDigit)
	{
		currentCommaPosition_ = Comma::_afterFirstDigit;
	}
	else if(currentCommaPosition_ == Comma::_noComma)
	{
		currentCommaPosition_ = Comma::_afterSecondDigit;
	}
}


template <typename TRotaryEncoder, typename TIoPin, typename TEventLoop>
void Encoder<TRotaryEncoder, TIoPin, TEventLoop>::
shiftCommaRight(void) const
{
	if(currentCommaPosition_ == Comma::_afterFirstDigit)
	{
		currentCommaPosition_ = Comma::_afterSecondDigit;
	}
	else if(currentCommaPosition_ == Comma::_afterSecondDigit)
	{
		currentCommaPosition_ = Comma::_noComma;
	}
	else if(currentCommaPosition_ == Comma::_noComma)
	{
		currentCommaPosition_ = Comma::_afterFirstDigit;
	}
}


} /*namespace Component*/

#endif /* ENCODER_H_ */
