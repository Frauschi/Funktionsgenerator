#ifndef SUBMENUS_H_
#define SUBMENUS_H_

#include <string>
#include <cstring>
#include <math.h>
#include <limits>

#include "Display.h"
#include "SignalGenerationCommon.h"
#include "SystemManager.h"
#include "MenuBase.h"


namespace Interface
{

class SubmenuBase
{
public:
	typedef typename System::FourButtonArray::Button 		Button;
	typedef typename System::Display::TextSize 				TextSize;

	//Constructor
	SubmenuBase(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset);

	//Destructor
	virtual ~SubmenuBase() {};

	virtual void enterSubmenu(void) const = 0;
	virtual void printSummary(uint8_t xPos, uint8_t yPos) const = 0;

	std::string getName(void) const;
	std::string getButtonName(void) const;

	virtual void exitSubmenu(void) const;
	void toggleEncoderMode(void) const;

protected:
	const MenuBase& parent_;

	const std::string name_;
	const std::string buttonName_;
	const std::uint16_t eepromAdressOffset_;

	const System::Display& display_;
	const System::Encoder& encoder_;
	const System::FourButtonArray& displayButtons_;
	const System::EEPROM& eeprom_;

}; //Class SubmenuBase


SubmenuBase::
SubmenuBase(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset) :
	parent_(parent),
	name_(name),
	buttonName_(buttonName),
	eepromAdressOffset_(eepromAdressOffset),
	display_(System::instance().display()),
	encoder_(System::instance().encoder()),
	displayButtons_(System::instance().displayButtons()),
	eeprom_(System::instance().eeprom())
{

}


std::string SubmenuBase::
getName(void) const
{
	return name_;
}


std::string SubmenuBase::
getButtonName(void) const
{
	return buttonName_;
}


void SubmenuBase::
exitSubmenu(void) const
{
	encoder_.disable();
	display_.clearSubmenuArea();
}


//---------------------------------------------------------------------------------------
template <typename Tvalue>
class ValueSubmenu : public SubmenuBase
{
public:
	typedef typename MiscStuff::EncoderDigit				Digits;
	typedef typename MiscStuff::EncoderMode					Mode;
	typedef typename MiscStuff::EncoderCommaPosition		Comma;
	typedef typename System::Display::LineDirection			Direction;

	//Constructor
	ValueSubmenu(const MenuBase& parent,
				const std::string& name,
				const std::string& buttonName,
				const uint16_t eepromAdressOffset,
				const Tvalue value,
				const int8_t unitFactor,
				const std::string& unit,
				const Tvalue valueMinimum,
				const Tvalue valueMaximum);

	//Destructor
	~ValueSubmenu(void) {};

	//Functions
	void enterSubmenu(void) const override;
	void printSummary(uint8_t xPos, uint8_t yPos) const override;
	void exitSubmenu(void) const override;

	Tvalue getValue(void) const;
	Tvalue getMinValue(void) const;
	Tvalue getMaxValue(void) const;

	void saveBootValueToEEPROM(void) const;
	void setBootValue(Tvalue newValue) const;

	template<typename Tfunc>
	void setUpdateValueCallback(Tfunc&& func) const {
		updateValueFunction_ = std::forward<Tfunc>(func);
	}

	void callUpdateValueFunction(void) const;

private:
	mutable Tvalue value_;
	const Tvalue valueMinimum_;
	const Tvalue valueMaximum_;

	const int8_t unitFactor_; //Factor in which value is stored (e.g. milli = -3, kilo = 3, ...)
	const std::string unit_;

	//function will call corresponding function to update a value
	mutable std::function<void(Tvalue)> updateValueFunction_;

	void printValue(void) const;
	void printEncoder(void) const;
	void printRange(void) const;
	void checkRange(void) const;
	void updateValue(void) const;
};


template <typename Tvalue>
ValueSubmenu<Tvalue>::
ValueSubmenu(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset, const Tvalue value,
		const int8_t unitFactor, const std::string& unit, const Tvalue valueMinimum, const Tvalue valueMaximum) :
	SubmenuBase(parent, name, buttonName, eepromAdressOffset),
	value_(value),
	valueMinimum_(valueMinimum),
	valueMaximum_(valueMaximum),
	unitFactor_(unitFactor),
	unit_(unit),
	updateValueFunction_(nullptr)
{

}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
enterSubmenu(void) const
{
	display_.clearSubmenuArea();

// Reload Button Content and Button functions
	if(std::numeric_limits<Tvalue>::is_signed)
	{
		display_.reloadButtonContent("* -1","* 10",": 10","Back");
		displayButtons_.addHandlerForButton(Button::_1, [&]() {
			encoder_.toggleSign();
			updateValue();
		});
	}
	else
	{
		display_.reloadButtonContent("","* 10",": 10","Back");
		displayButtons_.addHandlerForButton(Button::_1, nullptr);
	}

	displayButtons_.addHandlerForButton(Button::_2, [&]() {
		encoder_.multiplyByTen();
		updateValue();
	});

	displayButtons_.addHandlerForButton(Button::_3, [&]() {
		encoder_.divideByTen();
		updateValue();
	});

	displayButtons_.addHandlerForButton(Button::_4, [&]() {
		exitSubmenu();
		parent_.enterMenu();
	});

	//Reload Encoder functions
	encoder_.addPressedHandler( [&]() {
		encoder_.toggleMode();
		updateValue();
	});

	encoder_.addRotateLeftHandler( [&]() {
		encoder_.leftTurn();
		updateValue();
	});

	encoder_.addRotateRightHandler( [&]() {
		encoder_.rightTurn();
		updateValue();
	});

	//print current state of Value Menu
	display_.printText(10, headerHeight+10, name_, TextSize::medium);
	encoder_.calculateDigitsFromValue(value_, unitFactor_);
	printValue();
	printEncoder();
	printRange();

	encoder_.enable(std::numeric_limits<Tvalue>::is_signed);

	display_.drawMenuBorder();
	display_.reloadContent();
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
printSummary(uint8_t const xPos, uint8_t const yPos) const
{
	display_.printText(xPos, yPos, name_, TextSize::small);
	uint8_t unitXcoord = display_.printNumber(xPos + xValueShift, yPos, value_, unitFactor_, TextSize::small, 2);
	display_.printText(unitXcoord, yPos, unit_, TextSize::small);
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
exitSubmenu(void) const
{
	/* Clear callbacks */
	encoder_.addRotateRightHandler(nullptr);
	encoder_.addRotateLeftHandler(nullptr);
	encoder_.addPressedHandler(nullptr);

	encoder_.disable();
	display_.clearSubmenuArea();
	saveBootValueToEEPROM();
}


template <typename Tvalue>
Tvalue ValueSubmenu<Tvalue>::
getValue(void) const
{
	return value_;
}


template <typename Tvalue>
Tvalue ValueSubmenu<Tvalue>::
getMinValue(void) const
{
	return valueMinimum_;
}


template <typename Tvalue>
Tvalue ValueSubmenu<Tvalue>::
getMaxValue(void) const
{
	return valueMaximum_;
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
saveBootValueToEEPROM(void) const
{
	eeprom_.saveValue((parent_.getEepromBaseAdress() + eepromAdressOffset_), value_, nullptr);
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
setBootValue(Tvalue newValue) const
{
	if(newValue > valueMaximum_)
	{
		value_ = valueMaximum_;
	}
	else if(newValue < valueMinimum_)
	{
		value_ = valueMinimum_;
	}
	else
	{
		value_ = newValue;
	}
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
printValue(void) const
{
	uint8_t unitXcoord = 0;

	display_.printSign(xPosValueSign, yPosValue, encoder_.getSign(), TextSize::medium);

	unitXcoord = display_.printDigits(xPosValuedigit1, yPosValue, encoder_.getDigit(Digits::_firstDigit), encoder_.getDigit(Digits::_secondDigit),
			encoder_.getDigit(Digits::_thirdDigit), encoder_.getComma(), encoder_.getFactor(), TextSize::medium);
	display_.printText(unitXcoord, yPosValue, unit_, TextSize::medium);
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
printEncoder(void) const
{
	uint8_t digit2Shift = 0;
	uint8_t digit3Shift = 0;

	switch(encoder_.getComma())
	{
		case Comma::_afterFirstDigit: digit2Shift = 5; digit3Shift = 5; break;
		case Comma::_afterSecondDigit: digit3Shift = 5; break;
		default: break;
	}

	if(encoder_.getMode() == Mode::_changePosition)
	{
		switch(encoder_.getPosition())
		{
		case Digits::_firstDigit: 	display_.drawLine(xPosValuedigit1, yPosValue+14, 7, Direction::horizontal); break;
		case Digits::_secondDigit: 	display_.drawLine(xPosValuedigit2 + digit2Shift, yPosValue+14, 7, Direction::horizontal); break;
		case Digits::_thirdDigit: 	display_.drawLine(xPosValuedigit3 + digit3Shift, yPosValue+14, 7, Direction::horizontal); break;
		default: break;
		}
	}
	else if(encoder_.getMode() == Mode::_changeDigit)
	{
		switch(encoder_.getPosition())
		{
		case Digits::_firstDigit: 	display_.invertArea(xPosValuedigit1-1, xPosValuedigit1+7, yPosValue, yPosValue+11); break;
		case Digits::_secondDigit: 	display_.invertArea(xPosValuedigit2-1 + digit2Shift, xPosValuedigit2+7 + digit2Shift, yPosValue, yPosValue+11); break;
		case Digits::_thirdDigit: 	display_.invertArea(xPosValuedigit3-1 + digit3Shift, xPosValuedigit3+7 + digit3Shift, yPosValue, yPosValue+11); break;
		default: break;
		}
	}
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
printRange(void) const
{
	uint8_t nextXCoordinate = 0;

	display_.printText(10, headerHeight+66, "Range:");
	nextXCoordinate = display_.printNumber(60, headerHeight+66, valueMinimum_, unitFactor_, TextSize::small, 0);
	nextXCoordinate = display_.printText(nextXCoordinate, headerHeight+66, unit_);
	nextXCoordinate = display_.printText(nextXCoordinate+10, headerHeight+66, "to");
	nextXCoordinate = display_.printNumber(nextXCoordinate+10, headerHeight+66, valueMaximum_, unitFactor_, TextSize::small, 0);
	display_.printText(nextXCoordinate, headerHeight+66, unit_);
	nextXCoordinate = display_.printText(170, headerHeight+66, "(Steps:");
	nextXCoordinate = display_.printNumber(nextXCoordinate+5, headerHeight+66, static_cast<uint32_t>(1), unitFactor_, TextSize::small, 0);
	nextXCoordinate = display_.printText(nextXCoordinate, headerHeight+66, unit_);
	nextXCoordinate = display_.printText(nextXCoordinate, headerHeight+66, ")");
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
checkRange(void) const
{
	if(value_ > valueMaximum_)
	{
		value_ = valueMaximum_;
		encoder_.calculateDigitsFromValue(value_, unitFactor_);
	}
	else if(value_ < valueMinimum_)
	{
		value_ = valueMinimum_;
		encoder_.calculateDigitsFromValue(value_, unitFactor_);
	}
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
updateValue(void) const
{
	value_ = encoder_.calculateValueFromDigits<Tvalue>(unitFactor_);
	checkRange();
	callUpdateValueFunction();
	display_.clearSubmenuValue();
	printValue();
	printEncoder();
	display_.reloadContent();
}


template <typename Tvalue>
void ValueSubmenu<Tvalue>::
callUpdateValueFunction(void) const
{
	if(updateValueFunction_)
	{
		/* Disable the Encoder while updating the value
		 * This is necessary because when updating the frequency, an encoder-interrupt
		 * is triggered by starting an I2C transmission. */
		encoder_.disable();
		updateValueFunction_(value_);
		encoder_.enable(std::numeric_limits<Tvalue>::is_signed);
	}
}


//---------------------------------------------------------------------------------------
class FormSubmenu : public SubmenuBase
{
public:
	typedef typename System::Display::LineDirection			Direction;
	//Constructor
	FormSubmenu(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset, SignalGeneration::Waveform const form);
	//Destructor
	~FormSubmenu(void) {};

	void enterSubmenu(void) const override;
	void printSummary(uint8_t const xPos, uint8_t const yPos) const override;
	void exitSubmenu(void) const override;

	SignalGeneration::Waveform getWaveform(void) const;

	void saveBootValueToEEPROM(void) const;
	void setBootForm(SignalGeneration::Waveform form) const;

	template<typename Tfunc>
	void setUpdateWaveformCallback(Tfunc&& func) const {
		updateWaveformFunction_ = std::forward<Tfunc>(func);
	}

	void callUpdateWaveformFunction(void) const;

private:
	//function will call corresponding function to update the waveform
	mutable std::function<void(SignalGeneration::Waveform)> updateWaveformFunction_;

	mutable SignalGeneration::Waveform currentForm_;

	void reloadFormString(void) const;
	mutable std::string currentFormString_;

	void printWaveform(void) const;
}; //Class FormSubmenu


FormSubmenu::
FormSubmenu(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset, SignalGeneration::Waveform const form) :
	SubmenuBase(parent, name, buttonName, eepromAdressOffset),
	updateWaveformFunction_(nullptr),
	currentForm_(form)
{
	reloadFormString();
}


void FormSubmenu::
enterSubmenu(void) const
{
	display_.clearSubmenuArea();

	// Reload Button Content and Button functions
	display_.reloadButtonContent("","","","Back");
	displayButtons_.addHandlerForButton(Button::_1, nullptr);

	displayButtons_.addHandlerForButton(Button::_2, nullptr);

	displayButtons_.addHandlerForButton(Button::_3, nullptr);

	displayButtons_.addHandlerForButton(Button::_4, [&]() {
		exitSubmenu();
		parent_.enterMenu();
	});

	//print Name
	display_.printText(10, headerHeight+10, name_, TextSize::medium);

	printWaveform();

	encoder_.addPressedHandler(nullptr);

	encoder_.addRotateLeftHandler( [this]() {
		switch(currentForm_)
		{
			case SignalGeneration::Waveform::Sine: 		currentForm_ = SignalGeneration::Waveform::Saw_pos;  	break;
			case SignalGeneration::Waveform::Rect: 		currentForm_ = SignalGeneration::Waveform::Sine;		break;
			case SignalGeneration::Waveform::Triangle:	currentForm_ = SignalGeneration::Waveform::Rect; 		break;
			case SignalGeneration::Waveform::Saw_neg:	currentForm_ = SignalGeneration::Waveform::Triangle; 	break;
			case SignalGeneration::Waveform::Saw_pos: 	currentForm_ = SignalGeneration::Waveform::Saw_neg; 	break;
			default: break;
		}
		printWaveform();
		callUpdateWaveformFunction();
	});

	encoder_.addRotateRightHandler( [this]() {
		switch(currentForm_)
		{
			case SignalGeneration::Waveform::Sine: 		currentForm_ = SignalGeneration::Waveform::Rect; 		break;
			case SignalGeneration::Waveform::Rect: 		currentForm_ = SignalGeneration::Waveform::Triangle; 	break;
			case SignalGeneration::Waveform::Triangle:	currentForm_ = SignalGeneration::Waveform::Saw_neg; 	break;
			case SignalGeneration::Waveform::Saw_neg:	currentForm_ = SignalGeneration::Waveform::Saw_pos; 	break;
			case SignalGeneration::Waveform::Saw_pos: 	currentForm_ = SignalGeneration::Waveform::Sine;		break;
			default: break;
		}
		printWaveform();
		callUpdateWaveformFunction();
	});

	encoder_.enable(false);

	display_.drawMenuBorder();
	display_.reloadContent();
}


void FormSubmenu::
exitSubmenu(void) const
{
	/* Clear callbacks */
	encoder_.addRotateRightHandler(nullptr);
	encoder_.addRotateLeftHandler(nullptr);
	encoder_.addPressedHandler(nullptr);

	encoder_.disable();
	display_.clearSubmenuArea();
	saveBootValueToEEPROM();
}


void FormSubmenu::
printSummary(uint8_t const xPos, uint8_t const yPos) const
{
	display_.printText(xPos, yPos, name_);
	display_.printText(xPos + xValueShift, yPos, currentFormString_);
}


void FormSubmenu::
saveBootValueToEEPROM(void) const
{
	eeprom_.saveValue((parent_.getEepromBaseAdress() + eepromAdressOffset_), currentForm_, nullptr);
}


void FormSubmenu::
setBootForm(SignalGeneration::Waveform form) const
{
	currentForm_ = form;
	reloadFormString();
}


SignalGeneration::Waveform FormSubmenu::
getWaveform(void) const
{
	return currentForm_;
}


void FormSubmenu::
reloadFormString(void) const
{
	switch(currentForm_)
	{
		case SignalGeneration::Waveform::Sine: 		currentFormString_ = "Sinewave"; 	break;
		case SignalGeneration::Waveform::Rect: 		currentFormString_ = "Rectangle"; 	break;
		case SignalGeneration::Waveform::Triangle:	currentFormString_ = "Triangle"; 	break;
		case SignalGeneration::Waveform::Saw_neg:	currentFormString_ = "Saw Falling"; break;
		case SignalGeneration::Waveform::Saw_pos: 	currentFormString_ = "Saw Rising"; 	break;
		default: break;
	}
}


void FormSubmenu::
printWaveform(void) const
{
	display_.clearSubmenuValue();
	reloadFormString();
	display_.printText(80, yPosValue, currentFormString_, TextSize::medium);
	display_.invertArea(79, 87, yPosValue, yPosValue+11);
	display_.reloadContent();
}


void FormSubmenu::
callUpdateWaveformFunction(void) const
{
	if(updateWaveformFunction_)
	{
		updateWaveformFunction_(currentForm_);
	}
}


//---------------------------------------------------------------------------------------
class CreditsSubmenu : public SubmenuBase
{
public:
	//Constructor
	CreditsSubmenu(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset);
	//Destructor
	~CreditsSubmenu(void);

	void enterSubmenu(void) const override;
	void printSummary(uint8_t const xPos, uint8_t const yPos) const override;

private:

	const std::string title;
	mutable std::string nameTobi;
	const std::string nameTom;
	const std::string nameManu;
	const std::string subject;

}; //Class CreditsSubmenu


CreditsSubmenu::
CreditsSubmenu(const MenuBase& parent, const std::string& name, const std::string& buttonName, const uint16_t eepromAdressOffset) :
	SubmenuBase(parent, name, buttonName, eepromAdressOffset),
	title("WAVEFORMGENERATOR 2.0"),
	nameTobi("Frauenschlager Tobias"),
	nameTom("Taugenbeck Thomas"),
	nameManu("Dentgen Manuel"),
	subject("VMCB/VMS Wintersemester 2018/2019")
{
	nameTobi[10] = 0xE4; // change a to ä
}


CreditsSubmenu::
~CreditsSubmenu(void)
{

}


void CreditsSubmenu::
enterSubmenu(void) const
{
	display_.clearSubmenuArea();

	display_.reloadButtonContent("","","","Back");
	displayButtons_.addHandlerForButton(Button::_1, nullptr);
	displayButtons_.addHandlerForButton(Button::_2, nullptr);
	displayButtons_.addHandlerForButton(Button::_3, nullptr);
	displayButtons_.addHandlerForButton(Button::_4, [&]() {
		exitSubmenu();
		parent_.enterMenu();
	});

	display_.reloadHeaderContent(title);

	display_.printText(30, 30, subject, TextSize::small);
	display_.printText(120 - ( ( nameTobi.length() * 5) / 2), 50, nameTobi, TextSize::small);
	display_.printText(120 - ( ( nameTom.length() * 5) / 2), 70, nameTom, TextSize::small);
	display_.printText(120 - ( ( nameManu.length() * 5) / 2), 90, nameManu, TextSize::small);

	/* print all letters, digits and signs in creditsmenu (for visual Testing of DisplayLetters.h) */

//	display_.printText(10, 40, "abcd efgh ijkl mnop qrst uvwx yz", TextSize::small);
//	display_.printText(10, 50, "ABCD EFGH IJKL MNOP QRST UVWX YZ", TextSize::small);
//	display_.printText(10, 60, "���/() 0123 4567 89%= .:+- *,���", TextSize::small);
//
//	display_.printText(10, 70, "abcd efgh ijkl mnop qrst uvwx yz", TextSize::medium);
//	display_.printText(10, 82, "ABCD EFGH IJKL MNOP QRST UV", TextSize::medium);
//	display_.printText(10, 94, "WXYZ 0123 4567 89%. ,���-| ���", TextSize::medium);

	display_.drawMenuBorder();
	display_.reloadContent();
}


void CreditsSubmenu::
printSummary(uint8_t const xPos, uint8_t const yPos) const
{
	display_.printText(50+xPos, yPos, title, TextSize::small);
	display_.printText(20+xPos, yPos + yMenuNameShift, subject, TextSize::small);
}


} //Namespace Interface

#endif	//SUBMENUS_H_
