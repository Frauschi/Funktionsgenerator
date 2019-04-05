#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <DisplayLetters.h>
#include <cstdint>
#include <string>
#include <cstring>
#include <math.h>
#include <chrono>

using namespace std::chrono_literals;

namespace Component
{

#define xSize					240
#define ySize					64

#define buttonHeight			18
#define headerHeight			24

#define textHeightSmall			7
#define textHeightMedium		9
#define textHeightLarge			11

#define xPosBTN1center			28
#define xPosBTN2center			88
#define xPosBTN3center			148
#define xPosBTN4center			208
#define xPosMenuName			10
#define xPosMenuActivation		110
#define xPosMenuPage			175

#define xValueShift				165
#define yMenuNameShift			12

#define xPosValueSign			143
#define xPosValuedigit1			150
#define xPosValuedigit2			159
#define xPosValuedigit3			168
#define yPosValue				headerHeight+30

#define AsciiOffset				0x30

// Display settings
#define DSPsetView				0xC0
#define DSPbottomView			0x02
#define DSPtopView				0x04

#define DSPsetContrast			0x81
//#define DSPContrastValue		0x8F

#define DSPdisableAllPxlOn		0xA4
#define DSPenableAllPxlOn		0xA5
#define DSPdisableInverse 		0xA6
#define DSPenableInverse		0xA7
#define DSPenableLCD1shade		0xA9
#define DSPenableLCD4shade		0xAD
#define DSPenableLCD8shade		0xAB
#define DSPenableLCD16shade		0xAF

#define DSPlineRate9k4			0xA3

#define DSPbitPattern4bit		0xD0	//4bit per pixel

#define DSPsetSramPageAdressLSB	0x60
#define DSPsetSramPageAdressMSB	0x70
#define DSPsetStartColumnLSB	0x00
#define DSPsetStartColumnMSB	0x10

template <typename TSpiDriver, typename TBackgroundColorManager>
class Display
{
public:

	typedef typename TSpiDriver::DataHandling 				DataHandling;
	typedef typename MiscStuff::EncoderCommaPosition		CommaPosition;
	typedef typename MiscStuff::EncoderValueSign			Sign;

	// Constructor
	Display(const TSpiDriver& spiDriver, const TBackgroundColorManager& backgroundColor);
	// Destructor
	~Display();

	enum LineDirection{horizontal, vertical};
	enum LineBrightness{gray = 0x44, darkgray = 0x88, black = 0xCC};
	enum TextSize{small, medium, large};

	void reset(void) const;
	void init(void) const;

//	bool isInitialized(void) const;

	void drawLine(uint8_t const xCoordinate, uint8_t const yCoordinate, uint8_t const length, LineDirection const direction,
			LineBrightness const brightness = black) const;
	//void drawLine(uint8_t xCoordinateStart, uint8_t yCoordinateStart, uint8_t xCoordinateEnd, uint8_t yCoordinateEnd, LineBrightness brightness = black);
	//void drawRectangle(uint8_t xCoordinate, uint8_t yCoordinate, uint8_t length, uint8_t height);

	/**	 draw a Text to a specific position in the local display Content (will not send Commands to Display!)
	 * 	!! Warning !! Care that the xCoordinate and yCoordinate describe the left bottom pixel of the first letter
	 * 	@param xCoordinate - starting xCoordinate to print the Text (left bottom pixel of the first letter)
	 * 	@param yCoordinate - starting yCoordinate to print the Text (left bottom pixel of the first letter)
	 * 	@param textptr  - reference to the text which should be printed
	 * 	@param size - Text Size (small, medium)
	 *
	 * 	@retval a new xCoordinate after Last letter of 'textptr'
	 */
	uint8_t printText(uint8_t const xCoordinate, uint8_t const yCoordinate, const std::string& textptr, TextSize const size = small) const;

	/** draw a Number to a specific posotion in the local display Content (will not send Commands to Display!)
	 *  will also draw the factor of the number (e.g. k for kilo(10^3)
	 *  and will also draw decimals if necessary
	 *
	 *	@param xCoordinate - starting xCoordinate to print the Text
	 * 	@param yCoordinate - starting yCoordinate to print the Text
	 * 	@param number - the number which will be drawn
	 * 	@param factor - the corresponding factor to the number (e.g. -3 if number is a milli Value)
	 * 	@param size - Text Size (small, medium)
	 * 	@param maxAmountOfDecimals - if number has decimal digits this will be the max amount drawn
	 *
	 * 	@retval a new xCoordinate after the number or the factor Letter
	 */
	uint8_t printNumber(uint8_t const xCoordinate, uint8_t const yCoordinate, uint32_t const number, int8_t const factor,
			TextSize const size = small, uint8_t const maxAmountOfDecimals = 2) const;
	uint8_t printNumber(uint8_t const xCoordinate, uint8_t const yCoordinate, int32_t const number, int8_t const factor,
			TextSize const size = small, uint8_t const maxAmountOfDecimals = 2) const;
	uint8_t printSign(uint8_t const xCoordinate, uint8_t const yCoordinate, Sign const sign, TextSize const size = small) const;
	uint8_t printDigits(uint8_t const xCoordinate, uint8_t const yCoordinate, uint8_t const firstDigit,
				uint8_t const secondDigit, uint8_t const thirdDigit, CommaPosition const comma, int8_t const factor, TextSize const size = small) const;
	void printActivationSign(uint8_t const xCoordinate, uint8_t const yCoordinate, bool const activated) const;

	void drawMenuBorder(void) const;
	void reloadHeaderContent(const std::string& menuName, bool const activated, uint8_t const currentPage, uint8_t const amountOfPages,
			bool writeToDisplay = false) const;
	void reloadHeaderContent(const std::string& menuName, bool writeToDisplay = false) const;
	void reloadButtonContent(const std::string& Button1, const std::string& Button2, const std::string& Button3,
			const std::string& Button4, bool writeToDisplay = false) const;
	//void reloadSubmenuContent(const std::string& subMenuName, const std::string& subMenuValue, const std::string& subMenuUnit, bool writeToDisplay = false);

	void clearArea(uint8_t xPosStart, uint8_t xPosEnd, uint8_t yPosStart, uint8_t yPosEnd) const;
	void clearSubmenuArea(void) const;
	void clearSubmenuValue(void) const;
	void clearContent(void) const;
	void invertArea(uint8_t xPosStart, uint8_t xPosEnd, uint8_t yPosStart, uint8_t yPosEnd) const;

//	void reloadArea(uint8_t xPosStart, uint8_t xPosEnd, uint8_t yPosStart, uint8_t yPosEnd) const;
	void reloadRows(uint8_t StartRow, uint8_t EndRow) const;
	void reloadContent(void) const;

	size_t printLetter(uint8_t const xCoordinate, uint8_t const yCoordinate, const char letter, TextSize const size = small) const;
	size_t printDigit(uint8_t const xCoordinate, uint8_t const yCoordinate, uint8_t const digit, TextSize const size = small) const;

	void setBGred(uint8_t const colorValue) const;
	void setBGgreen(uint8_t const colorValue) const;
	void setContrast(uint8_t const contrast) const;

private:
	uint8_t getSpaceValue(TextSize const size) const;
	bool checkBoundary(uint8_t xPosEnd, uint8_t yPosEnd) const;

	uint8_t** displayContent_;
	static constexpr size_t contentArraySize = xSize*ySize;

	const TSpiDriver& spiDriver_;
	const TBackgroundColorManager& backgroundColor_;
};


template <typename TSpiDriver, typename TBackgroundColorManager>
Display<TSpiDriver, TBackgroundColorManager>::
Display(const TSpiDriver& spiDriver, const TBackgroundColorManager& backgroundColor) :
	spiDriver_(spiDriver),
	backgroundColor_(backgroundColor)
{
	uint8_t* rawBuffer = reinterpret_cast<uint8_t*>(malloc(contentArraySize));

	displayContent_ = new uint8_t*[ySize];
	if(!displayContent_)
	{
		//while(1){}; //allocating new memory failed
	}

	for(size_t i = 0; i<xSize; i++)
	{
		displayContent_[i] = new(rawBuffer + i*xSize) uint8_t[xSize];
		if(!displayContent_[i])
		{
			//while(1){}; //allocating new memory failed
		}
	}

	clearContent();

	backgroundColor_.setRedColor(100);
	backgroundColor_.setGreenColor(100);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
Display<TSpiDriver, TBackgroundColorManager>::
~Display()
{
	for(size_t i = 0; i<xSize; i++)
	{
		delete(displayContent_[i]);
	}
	delete(displayContent_);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
reset(void) const
{
	std::uint8_t ResetCmd[] = {0xE2};
	spiDriver_.asyncWrite(ResetCmd, sizeof(ResetCmd), DataHandling::dspCommand, nullptr);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
init(void) const
{
	std::uint8_t const initCmds[] = {
								0xF1, 0x7F, 0xF2, 0x00, 0xF3, 0x7F,
								/*DSPsetContrast, DSPContrastValue,*/
								DSPsetView, DSPbottomView,
								DSPdisableAllPxlOn,  /*DSPenableAllPxlOn,*/
								DSPlineRate9k4,
								DSPbitPattern4bit,
								DSPdisableInverse, /*DSPenableInverse,*/
								DSPsetSramPageAdressLSB, DSPsetSramPageAdressMSB,
								DSPsetStartColumnLSB, DSPsetStartColumnMSB,
								/*DSPenableLCD1shade*/ DSPenableLCD4shade /*DSPenableLCD8shade*/ /*DSPenableLCD16shade*/};

	spiDriver_.asyncWrite(initCmds, sizeof(initCmds), DataHandling::dspCommand, nullptr);

	setContrast(50);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
uint8_t Display<TSpiDriver, TBackgroundColorManager>::
printText(uint8_t const xCoordinate, uint8_t const yCoordinate, const std::string& text, TextSize const size) const
{
	uint8_t xCoord = xCoordinate;
	uint8_t spaceValue = 0;

	spaceValue = getSpaceValue(size);

	for(const char& letter : text)
	{
		uint8_t lastWidth = printLetter(xCoord, (yCoordinate/2), letter, size);
		xCoord += (lastWidth + spaceValue);
	}

	return(xCoord);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
uint8_t Display<TSpiDriver, TBackgroundColorManager>::
printNumber(uint8_t const xCoordinate, uint8_t const yCoordinate, uint32_t const number, int8_t const factor,
		TextSize const size, uint8_t const maxAmountOfDecimals) const
{
	uint8_t xCoord = xCoordinate;
	uint8_t spaceValue = 0;

	uint8_t digits[3] = {0,0,0};		//prekomma
	uint8_t digitsDeci[2] = {0,0};		//decimal digits
	uint8_t printedDigits = 0;

	int8_t matchedFactor = factor;
	char factorLetter = ' ';
	uint32_t matchedNumber = number;
	uint8_t AmountOfDecimals = maxAmountOfDecimals;

	spaceValue = getSpaceValue(size);

	if(number == 0) //if current value is 0 dont go through the whole process
	{
		matchedFactor = 0;
		printedDigits = 2;
		digits[2] = 0;
		AmountOfDecimals = 0;
	}
	else
	{
		if(factor != 0)
		{
			matchedNumber = static_cast<uint32_t>(number*pow(10, factor));

			if(matchedNumber == 0)
			{
				matchedNumber = number;
			}
			else if(matchedNumber > 0 && matchedNumber < 1e3)
			{
				digitsDeci[0] = (number%1000) / 100;
				digitsDeci[1] = (number%100) / 10;

				matchedFactor += 3;
			}
		}
		else
		{
			if(matchedNumber >= 1e6)
			{
				digitsDeci[0] = (matchedNumber%1000000) / 100000;
				digitsDeci[1] = (matchedNumber%100000) / 10000;

				matchedFactor += 6;
				matchedNumber /= 1e6;
			}
			else if(matchedNumber >= 1e3)
			{
				digitsDeci[0] = (matchedNumber%1000) / 100;
				digitsDeci[1] = (matchedNumber%100) / 10;

				matchedFactor += 3;
				matchedNumber /= 1e3;
			}
		}
		digits[0] = matchedNumber/100;
		digits[1] = (matchedNumber%100)/10;
		digits[2] = (matchedNumber%10);

		if(digits[0] != 0)
		{
			xCoord += (printDigit(xCoord, (yCoordinate/2), digits[0], size) + spaceValue);
			printedDigits ++;
		}
		if(digits[1] != 0 || digits[0]!= 0)
		{
			xCoord += (printDigit(xCoord, (yCoordinate/2), digits[1], size) + spaceValue);
			printedDigits ++;
		}
	}

	xCoord += (printDigit(xCoord, (yCoordinate/2), digits[2], size) + spaceValue);
	printedDigits ++;

	if(printedDigits < 3 && AmountOfDecimals != 0 && matchedFactor != factor)
	{
		AmountOfDecimals = 3 - printedDigits;
		xCoord += (printLetter(xCoord, (yCoordinate/2), ',', size) + spaceValue);

		for(uint8_t i = 0; i < AmountOfDecimals; i++)
		{
			xCoord += (printDigit(xCoord, (yCoordinate/2), digitsDeci[i], size) + spaceValue);
			if(i + printedDigits == 3)
			{
				break;
			}
		}
	}

	switch(matchedFactor)
	{
	case 6: 	factorLetter = 'M'; break;
	case 3: 	factorLetter = 'k'; break;
	case 0: 	factorLetter = ' '; break;
	case -3: 	factorLetter = 'm'; break;
	default: break;
	}

	if(factorLetter != ' ')
	{
		xCoord += 2;
		xCoord += (printLetter(xCoord, (yCoordinate/2), factorLetter, size) + spaceValue);
	}
	else
	{
		xCoord += 3;
	}

	return xCoord;
}


template <typename TSpiDriver, typename TBackgroundColorManager>
uint8_t Display<TSpiDriver, TBackgroundColorManager>::
printNumber(uint8_t const xCoordinate, uint8_t const yCoordinate, int32_t const number, int8_t const factor,
		TextSize const size, uint8_t const maxAmountOfDecimals) const
{
	uint8_t spaceValue = 0;
	uint8_t xCoord = xCoordinate;
	uint32_t AbsoluteNumber = 0;

	spaceValue = getSpaceValue(size);

	if(number<0)
	{
		xCoord += (printLetter(xCoord, (yCoordinate/2), '-', size) + spaceValue);
	}

	AbsoluteNumber = abs(number);
	xCoord = printNumber(xCoord, yCoordinate, AbsoluteNumber, factor, size, maxAmountOfDecimals);

	return(xCoord);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
uint8_t Display<TSpiDriver, TBackgroundColorManager>::
printSign(uint8_t const xCoordinate, uint8_t const yCoordinate, Sign const sign, TextSize const size) const
{
	uint8_t xCoord = xCoordinate;
	uint8_t spaceValue = 0;
	spaceValue = getSpaceValue(size);

	if(sign == Sign::_negative)
	{
		xCoord += (printLetter(xCoord, (yCoordinate/2), '-', size) + spaceValue);
	}

	return(xCoord);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
uint8_t Display<TSpiDriver, TBackgroundColorManager>::
printDigits(uint8_t const xCoordinate, uint8_t const yCoordinate, uint8_t const firstDigit, uint8_t const secondDigit,
		uint8_t const thirdDigit, CommaPosition const commaPosition, int8_t const factor, TextSize const size) const
{
	uint8_t xCoord = xCoordinate;
	uint8_t spaceValue = 0;

	int8_t matchedFactor = factor;
	char factorLetter = ' ';

	uint8_t matchedFirstDigit;
	uint8_t matchedSecondDigit;

	if(firstDigit == 0 && commaPosition != CommaPosition::_afterFirstDigit)
	{
		matchedFirstDigit = 255;
	}
	else
	{
		matchedFirstDigit = firstDigit;
	}

	if(firstDigit == 0 && secondDigit == 0 && commaPosition == CommaPosition::_noComma)
	{
		matchedSecondDigit = 255;
	}
	else
	{
		matchedSecondDigit = secondDigit;
	}

	spaceValue = getSpaceValue(size);

	xCoord += (printDigit(xCoord, (yCoordinate/2), matchedFirstDigit, size) + spaceValue);
	if(commaPosition == CommaPosition::_afterFirstDigit)
	{
		xCoord += (printLetter(xCoord, (yCoordinate/2), ',', size) + spaceValue);
	}
	xCoord += (printDigit(xCoord, (yCoordinate/2), matchedSecondDigit, size) + spaceValue);
	if(commaPosition == CommaPosition::_afterSecondDigit)
	{
		xCoord += (printLetter(xCoord, (yCoordinate/2), ',', size) + spaceValue);
	}
	xCoord += (printDigit(xCoord, (yCoordinate/2), thirdDigit, size) + spaceValue);

	switch(matchedFactor)
		{
		case 6: 	factorLetter = 'M'; break;
		case 3: 	factorLetter = 'k'; break;
		case 0: 	factorLetter = ' '; break;
		case -3: 	factorLetter = 'm'; break;
		default: break;
		}

		if(factorLetter != ' ')
		{
			xCoord += 2;
			xCoord += (printLetter(xCoord, (yCoordinate/2), factorLetter, size) + spaceValue);
		}
		else
		{
			xCoord += 3;
		}

	return(xCoord);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
printActivationSign(uint8_t const xCoordinate, uint8_t const yCoordinate, bool const activated) const
{
	if(activated)
	{
		printText(xCoordinate, yCoordinate, "Active", small);
	}
	else
	{
		printText(xCoordinate, yCoordinate, "Inactive", small);
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
reloadHeaderContent(const std::string& menuName, bool const activated, uint8_t const currentPage,
		uint8_t const amountOfPages, bool writeToDisplay) const
{
	clearArea(1, xSize-2, 1, headerHeight - 1);

	printText(xPosMenuName, headerHeight - 16, menuName, medium);
	printActivationSign(xPosMenuActivation, headerHeight - 12, activated);
	printText(xPosMenuPage, headerHeight - 12, "PAGE:", small);
	printNumber(xPosMenuPage+30, headerHeight - 12, static_cast<uint32_t>(currentPage), 0, small, 0);
	printText(xPosMenuPage+37, headerHeight - 12,"/", small);
	printNumber(xPosMenuPage+44, headerHeight - 12, static_cast<uint32_t>(amountOfPages), 0, small, 0);

	if(writeToDisplay) {
		reloadRows(0, headerHeight);
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
reloadHeaderContent(const std::string& menuName, bool writeToDisplay) const
{
	clearArea(1, xSize-2, 1, headerHeight - 1);

	printText(xPosMenuName, headerHeight - 16, menuName, medium);

	if(writeToDisplay) {
		reloadRows(0, headerHeight);
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
reloadButtonContent(const std::string& Button1, const std::string& Button2, const std::string& Button3, const std::string& Button4,
		bool writeToDisplay) const
{
	clearArea(1, 58, ySize*2 - (buttonHeight + 1), 125);
	printText(xPosBTN1center - ((uint8_t)(Button1.length() * (float) 5.2 ) / 2) - 1, 114, Button1, small);

	clearArea(60, 118, ySize*2 - (buttonHeight + 1), 125);
	printText(xPosBTN2center - ((uint8_t)(Button2.length() * (float) 5.2 ) / 2) - 1, 114, Button2, small);

	clearArea(120, 178, ySize*2 - (buttonHeight + 1), 125);
	printText(xPosBTN3center - ((uint8_t)(Button3.length() * (float) 5.2 ) / 2) - 1, 114, Button3, small);

	clearArea(180, 238, ySize*2 - (buttonHeight + 1), 125);
	printText(xPosBTN4center - ((uint8_t)(Button4.length() * (float) 5.2 ) / 2) - 1, 114, Button4, small);

	if(writeToDisplay) {
		reloadRows((ySize*2)-buttonHeight, (ySize*2)-1);
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
drawLine(uint8_t xCoordinate, uint8_t yCoordinate, uint8_t length, LineDirection const direction,
		LineBrightness const brightness) const
{
	if(direction == horizontal) {
		uint8_t lineType = 0x00;
		if(xCoordinate + length >= xSize)
		{
			length = xSize - (xCoordinate + 1);
		}

		if(yCoordinate%2 == 0)
		{
			lineType = (0x0F & brightness);
		}
		else
		{
			lineType = (0xF0 & brightness);
		}

		yCoordinate /= 2;

		for(uint8_t x = xCoordinate;  x < (length + xCoordinate); x++)
		{
			displayContent_[yCoordinate][x] |= lineType;
		}
	}
	else {
		if(yCoordinate%2 == 1)
		{
			yCoordinate /= 2;

			displayContent_[yCoordinate][xCoordinate]  |= (0xF0 & brightness);
			yCoordinate++;
		}
		else
		{
			yCoordinate /= 2;
		}

		length = length/2;

		if(yCoordinate + length >= ySize)
		{
			length = ySize - (yCoordinate + 1);
		}

		for(uint8_t y = yCoordinate; y <= (length + yCoordinate); y++)
		{
			displayContent_[y][xCoordinate] |= (0xFF & brightness);
		}
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
drawMenuBorder(void) const
{
	//outer Border
	drawLine(0, 0, (xSize-1), horizontal);
	drawLine(0, ((ySize*2)-1), (xSize-1), horizontal);
	drawLine(0, 0, ((ySize*2)-1), vertical);
	drawLine((xSize-1), 0, ((ySize*2)-1), vertical);

	//Headline
	drawLine(0, headerHeight, (xSize-1), horizontal);

	//Buttons
	drawLine(0, (ySize*2 - (buttonHeight+1)), (xSize-1), horizontal);
	drawLine(59, (ySize*2 - (buttonHeight+1)), buttonHeight, vertical);
	drawLine(119, (ySize*2 - (buttonHeight+1)), buttonHeight, vertical);
	drawLine(179, (ySize*2 - (buttonHeight+1)), buttonHeight, vertical);
}


//Clear functions only affect the content of the local displaycontent variable
//They do NOT send new display data
template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
clearArea(uint8_t xPosStart, uint8_t xPosEnd, uint8_t yPosStart, uint8_t yPosEnd) const
{
	if(checkBoundary(xPosEnd, yPosEnd))
	{
		if(yPosStart%2 == 1)
		{
			for(size_t j = xPosStart; j <= xPosEnd; j++)
			{
				displayContent_[yPosStart/2][j] &= 0x0F;
			}
		yPosStart = (yPosStart/2)+1;
		}
		else
		{
		yPosStart /= 2;
		}

		if(yPosEnd%2 == 1)
		{
			for(size_t j = xPosStart; j <= xPosEnd; j++)
			{
				displayContent_[yPosEnd/2][j] &= 0xF0;
			}
		}
		yPosEnd /= 2;

		for(size_t i = yPosStart; i <= yPosEnd; i++)
		{
			for(size_t j = xPosStart; j <= xPosEnd; j++)
			{
				displayContent_[i][j] = 0x00;
			}
		}
	}
	else
	{
		//do nothing
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
clearSubmenuArea(void) const
{
	clearArea(1, xSize-2, headerHeight+1, ySize*2 - (buttonHeight + 4));
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
clearSubmenuValue(void) const
{
	clearArea(1, xSize-2, headerHeight+30, headerHeight+60);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
clearContent(void) const
{
	clearArea(0,(xSize-1),0,((ySize*2)-1));
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
invertArea(uint8_t xPosStart, uint8_t xPosEnd, uint8_t yPosStart, uint8_t yPosEnd) const
{
	if(checkBoundary(xPosEnd, yPosEnd))
	{
		if(yPosStart%2 == 1)
		{
			for(size_t j = xPosStart; j <= xPosEnd; j++)
			{
				displayContent_[yPosStart/2][j] ^= 0xF0;
			}
		yPosStart = (yPosStart/2) + 1;
		}
		else
		{
		yPosStart /= 2;
		}

		if(yPosEnd%2 == 1)
		{
			for(size_t j = xPosStart; j <= xPosEnd; j++)
			{
				displayContent_[(yPosEnd+1)/2][j] ^= 0x0F;
			}
		}
		yPosEnd /= 2;

		for(size_t i = yPosStart; i <= yPosEnd; i++)
		{
			for(size_t j = xPosStart; j <= xPosEnd; j++)
			{
				displayContent_[i][j] ^= 0xFF;
			}
		}
	}
	else
	{
		//do nothing
	}
}

//Replaced ReloadArea function with ReloadRows function due to performance Reasons

//template <typename TSpiDriver, typename TBackgroundColorManager>
//void Display<TSpiDriver, TBackgroundColorManager>::
//reloadArea(uint8_t xPosStart, uint8_t xPosEnd, uint8_t yPosStart, uint8_t yPosEnd) const
//{
//	//change this to reloadRows
//	if(yPosStart%2 == 1)
//	{
//		yPosStart -= 1;
//	}
//	yPosStart /= 2;
//
//	if(yPosEnd%2 == 1)
//	{
//		if(yPosEnd < (ySize*2))
//		{
//			yPosEnd += 1;
//		}
//		else
//		{
//			yPosEnd -= 1;
//		}
//	}
//	yPosEnd /= 2;
//
//	for(uint8_t i = yPosStart; i <= yPosEnd; i++)
//	{
//		std::uint8_t positionCmds[] =
//		{
//			(uint8_t)(DSPsetStartColumnLSB | (xPosStart & 0x0F)),
//			(uint8_t)(DSPsetStartColumnMSB | ((xPosStart & 0xF0)>>4)),
//			(uint8_t)(DSPsetSramPageAdressLSB  | (i & 0x0F)),
//			(uint8_t)(DSPsetSramPageAdressMSB | ((i & 0x70)>>4))
//		};
//
//		spiDriver_.asyncWrite(positionCmds, sizeof(positionCmds), DataHandling::dspCommand, nullptr);
//
//		spiDriver_.asyncWrite(&displayContent_[i][xPosStart], (xPosEnd-xPosStart), DataHandling::dspData, nullptr);
//	}
//}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
reloadRows(uint8_t StartRow, uint8_t EndRow) const
{
	if(StartRow%2 == 1)
	{
		StartRow -= 1;
	}
	StartRow /= 2;

	if(EndRow%2 == 1)
	{
		if(EndRow < (ySize*2 + 1))
		{
			EndRow += 1;
			EndRow /= 2;
		}
		else
		{
			EndRow = (ySize-1);
		}
	}
	else
	{
		EndRow /= 2;
	}

	std::uint8_t positionCmds[] =
	{
		(uint8_t)(DSPsetSramPageAdressLSB  | (StartRow & 0x0F)),
		(uint8_t)(DSPsetSramPageAdressMSB | ((StartRow & 0x70)>>4)),
		DSPsetStartColumnLSB, DSPsetStartColumnMSB
	};

	spiDriver_.asyncWrite(positionCmds, sizeof(positionCmds), DataHandling::dspCommand, nullptr);

	spiDriver_.asyncWrite(&displayContent_[StartRow][0], (EndRow-StartRow)*(xSize-1), DataHandling::dspData, nullptr);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
reloadContent(void) const
{
	std::uint8_t initCmds[] =
	{
		DSPsetSramPageAdressLSB, DSPsetSramPageAdressMSB,
		DSPsetStartColumnLSB, DSPsetStartColumnMSB
	};

	spiDriver_.asyncWrite(initCmds, sizeof(initCmds), DataHandling::dspCommand, nullptr);

	spiDriver_.asyncWrite(&displayContent_[0][0], contentArraySize, DataHandling::dspData, nullptr);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
size_t Display<TSpiDriver, TBackgroundColorManager>::
printLetter(uint8_t const xCoordinate, uint8_t const yCoordinate, const char letter, TextSize const size) const
{
	size_t width = 0;

	if(size == small)
	{
		size_t height = 4;

		auto* letterPointer_ = DisplayLetters::letter_space_small;

		switch(letter)
		{
			case 'A': letterPointer_ = DisplayLetters::letter_A_small; width = 5; break;
			case 'B': letterPointer_ = DisplayLetters::letter_B_small; width = 5; break;
			case 'C': letterPointer_ = DisplayLetters::letter_C_small; width = 5; break;
			case 'D': letterPointer_ = DisplayLetters::letter_D_small; width = 5; break;
			case 'E': letterPointer_ = DisplayLetters::letter_E_small; width = 5; break;
			case 'F': letterPointer_ = DisplayLetters::letter_F_small; width = 5; break;
			case 'G': letterPointer_ = DisplayLetters::letter_G_small; width = 5; break;
			case 'H': letterPointer_ = DisplayLetters::letter_H_small; width = 5; break;
			case 'I': letterPointer_ = DisplayLetters::letter_I_small; width = 3; break;
			case 'J': letterPointer_ = DisplayLetters::letter_J_small; width = 5; break;
			case 'K': letterPointer_ = DisplayLetters::letter_K_small; width = 5; break;
			case 'L': letterPointer_ = DisplayLetters::letter_L_small; width = 5; break;
			case 'M': letterPointer_ = DisplayLetters::letter_M_small; width = 5; break;
			case 'N': letterPointer_ = DisplayLetters::letter_N_small; width = 5; break;
			case 'O': letterPointer_ = DisplayLetters::letter_O_small; width = 5; break;
			case 'P': letterPointer_ = DisplayLetters::letter_P_small; width = 5; break;
			case 'Q': letterPointer_ = DisplayLetters::letter_Q_small; width = 5; break;
			case 'R': letterPointer_ = DisplayLetters::letter_R_small; width = 5; break;
			case 'S': letterPointer_ = DisplayLetters::letter_S_small; width = 5; break;
			case 'T': letterPointer_ = DisplayLetters::letter_T_small; width = 5; break;
			case 'U': letterPointer_ = DisplayLetters::letter_U_small; width = 5; break;
			case 'V': letterPointer_ = DisplayLetters::letter_V_small; width = 5; break;
			case 'W': letterPointer_ = DisplayLetters::letter_W_small; width = 5; break;
			case 'X': letterPointer_ = DisplayLetters::letter_X_small; width = 5; break;
			case 'Y': letterPointer_ = DisplayLetters::letter_Y_small; width = 5; break;
			case 'Z': letterPointer_ = DisplayLetters::letter_Z_small; width = 5; break;

			case 'a': letterPointer_ = DisplayLetters::letter_a_small; width = 5; break;
			case 'b': letterPointer_ = DisplayLetters::letter_b_small; width = 5; break;
			case 'c': letterPointer_ = DisplayLetters::letter_c_small; width = 4; break;
			case 'd': letterPointer_ = DisplayLetters::letter_d_small; width = 5; break;
			case 'e': letterPointer_ = DisplayLetters::letter_e_small; width = 5; break;
			case 'f': letterPointer_ = DisplayLetters::letter_f_small; width = 5; break;
			case 'g': letterPointer_ = DisplayLetters::letter_g_small; width = 5; break;
			case 'h': letterPointer_ = DisplayLetters::letter_h_small; width = 5; break;
			case 'i': letterPointer_ = DisplayLetters::letter_i_small; width = 3; break;
			case 'j': letterPointer_ = DisplayLetters::letter_j_small; width = 4; break;
			case 'k': letterPointer_ = DisplayLetters::letter_k_small; width = 4; break;
			case 'l': letterPointer_ = DisplayLetters::letter_l_small; width = 3; break;
			case 'm': letterPointer_ = DisplayLetters::letter_m_small; width = 5; break;
			case 'n': letterPointer_ = DisplayLetters::letter_n_small; width = 5; break;
			case 'o': letterPointer_ = DisplayLetters::letter_o_small; width = 5; break;
			case 'p': letterPointer_ = DisplayLetters::letter_p_small; width = 5; break;
			case 'q': letterPointer_ = DisplayLetters::letter_q_small; width = 5; break;
			case 'r': letterPointer_ = DisplayLetters::letter_r_small; width = 5; break;
			case 's': letterPointer_ = DisplayLetters::letter_s_small; width = 5; break;
			case 't': letterPointer_ = DisplayLetters::letter_t_small; width = 5; break;
			case 'u': letterPointer_ = DisplayLetters::letter_u_small; width = 5; break;
			case 'v': letterPointer_ = DisplayLetters::letter_v_small; width = 5; break;
			case 'w': letterPointer_ = DisplayLetters::letter_w_small; width = 5; break;
			case 'x': letterPointer_ = DisplayLetters::letter_x_small; width = 5; break;
			case 'y': letterPointer_ = DisplayLetters::letter_y_small; width = 5; break;
			case 'z': letterPointer_ = DisplayLetters::letter_z_small; width = 5; break;
			case 0xE4: letterPointer_ = DisplayLetters::letter_ae_small; width = 5; break;

			case '0': letterPointer_ = DisplayLetters::digit_0_small; width = 5; break;
			case '1': letterPointer_ = DisplayLetters::digit_1_small; width = 3; break;
			case '2': letterPointer_ = DisplayLetters::digit_2_small; width = 5; break;
			case '3': letterPointer_ = DisplayLetters::digit_3_small; width = 5; break;
			case '4': letterPointer_ = DisplayLetters::digit_4_small; width = 5; break;
			case '5': letterPointer_ = DisplayLetters::digit_5_small; width = 5; break;
			case '6': letterPointer_ = DisplayLetters::digit_6_small; width = 5; break;
			case '7': letterPointer_ = DisplayLetters::digit_7_small; width = 5; break;
			case '8': letterPointer_ = DisplayLetters::digit_8_small; width = 5; break;
			case '9': letterPointer_ = DisplayLetters::digit_9_small; width = 5; break;

			case '/': letterPointer_ = DisplayLetters::sign_backslash_small; width = 4; break;
			case '%': letterPointer_ = DisplayLetters::sign_percent_small; width = 5; break;
			case '=': letterPointer_ = DisplayLetters::sign_equals_small; width = 4; break;
			case '.': letterPointer_ = DisplayLetters::sign_dot_small; width = 2; break;
			case ':': letterPointer_ = DisplayLetters::sign_colon_small; width = 2; break;
			case '-': letterPointer_ = DisplayLetters::sign_minus_small; width = 4; break;
			case '+': letterPointer_ = DisplayLetters::sign_plus_small; width = 3; break;
			case '#': letterPointer_ = DisplayLetters::sign_degree_small; width = 4; break;
			case '*': letterPointer_ = DisplayLetters::sign_asterisk_small; width = 3; break;
			case ',': letterPointer_ = DisplayLetters::sign_komma_small; width = 3; break;
			case '(': letterPointer_ = DisplayLetters::sign_brckt_open_small; width = 3; break;
			case ')': letterPointer_ = DisplayLetters::sign_brckt_close_small; width = 3; break;

			case ' ': width = 3; break;
			default: width = 3; break;
		}
		//include Letter to Display Content
		for(size_t y = 0; y < height; y++)
		{
			for(size_t x = 0; x < width; x++)
			{
				displayContent_[yCoordinate+y][xCoordinate+x] = letterPointer_[y][x];
			}
		}

	}
	else if(size == medium)
	{
		size_t height = 6;

		auto* letterPointer_ = DisplayLetters::letter_space_medium;

		switch(letter)
		{
			case 'A':	letterPointer_ = DisplayLetters::letter_A_medium; width = 7; break;
			case 'B':	letterPointer_ = DisplayLetters::letter_B_medium; width = 7; break;
			case 'C':  	letterPointer_ = DisplayLetters::letter_C_medium; width = 7; break;
			case 'D':  	letterPointer_ = DisplayLetters::letter_D_medium; width = 7; break;
			case 'E':	letterPointer_ = DisplayLetters::letter_E_medium; width = 7; break;
			case 'F':	letterPointer_ = DisplayLetters::letter_F_medium; width = 7; break;
			case 'G':	letterPointer_ = DisplayLetters::letter_G_medium; width = 7; break;
			case 'H':	letterPointer_ = DisplayLetters::letter_H_medium; width = 7; break;
			case 'I': 	letterPointer_ = DisplayLetters::letter_I_medium; width = 5; break;
			case 'J':	letterPointer_ = DisplayLetters::letter_J_medium; width = 5; break;
			case 'K': 	letterPointer_ = DisplayLetters::letter_K_medium; width = 7; break;
			case 'L':	letterPointer_ = DisplayLetters::letter_L_medium; width = 7; break;
			case 'M': 	letterPointer_ = DisplayLetters::letter_M_medium; width = 7; break;
			case 'N':	letterPointer_ = DisplayLetters::letter_N_medium; width = 7; break;
			case 'O':  	letterPointer_ = DisplayLetters::letter_O_medium; width = 7; break;
			case 'P':  	letterPointer_ = DisplayLetters::letter_P_medium; width = 7; break;
			case 'Q':  	letterPointer_ = DisplayLetters::letter_Q_medium; width = 7; break;
			case 'R':  	letterPointer_ = DisplayLetters::letter_R_medium; width = 7; break;
			case 'S':  	letterPointer_ = DisplayLetters::letter_S_medium; width = 7; break;
			case 'T':  	letterPointer_ = DisplayLetters::letter_T_medium; width = 7; break;
			case 'U':	letterPointer_ = DisplayLetters::letter_U_medium; width = 7; break;
			case 'V': 	letterPointer_ = DisplayLetters::letter_V_medium; width = 7; break;
			case 'W':  	letterPointer_ = DisplayLetters::letter_W_medium; width = 7; break;
			case 'X':  	letterPointer_ = DisplayLetters::letter_X_medium; width = 5; break;
			case 'Y': 	letterPointer_ = DisplayLetters::letter_Y_medium; width = 7; break;
			case 'Z': 	letterPointer_ = DisplayLetters::letter_Z_medium; width = 7; break;

			case 'a':	letterPointer_ = DisplayLetters::letter_a_medium; width = 5; break;
			case 'b':	letterPointer_ = DisplayLetters::letter_b_medium; width = 5; break;
			case 'c': 	letterPointer_ = DisplayLetters::letter_c_medium; width = 5; break;
			case 'd': 	letterPointer_ = DisplayLetters::letter_d_medium; width = 5; break;
			case 'e': 	letterPointer_ = DisplayLetters::letter_e_medium; width = 5; break;
			case 'f': 	letterPointer_ = DisplayLetters::letter_f_medium; width = 5; break;
			case 'g':	letterPointer_ = DisplayLetters::letter_g_medium; width = 5; break;
			case 'h': 	letterPointer_ = DisplayLetters::letter_h_medium; width = 5; break;
			case 'i': 	letterPointer_ = DisplayLetters::letter_i_medium; width = 3; break;
			case 'j': 	letterPointer_ = DisplayLetters::letter_j_medium; width = 3; break;
			case 'k': 	letterPointer_ = DisplayLetters::letter_k_medium; width = 5; break;
			case 'l': 	letterPointer_ = DisplayLetters::letter_l_medium; width = 3; break;
			case 'm':   letterPointer_ = DisplayLetters::letter_m_medium; width = 7; break;
			case 'n':   letterPointer_ = DisplayLetters::letter_n_medium; width = 5; break;
			case 'o':   letterPointer_ = DisplayLetters::letter_o_medium; width = 5; break;
			case 'p':   letterPointer_ = DisplayLetters::letter_p_medium; width = 5; break;
			case 'q':   letterPointer_ = DisplayLetters::letter_q_medium; width = 5; break;
			case 'r':   letterPointer_ = DisplayLetters::letter_r_medium; width = 5; break;
			case 's':   letterPointer_ = DisplayLetters::letter_s_medium; width = 5; break;
			case 't':   letterPointer_ = DisplayLetters::letter_t_medium; width = 5; break;
			case 'u':   letterPointer_ = DisplayLetters::letter_u_medium; width = 5; break;
			case 'v':   letterPointer_ = DisplayLetters::letter_v_medium; width = 5; break;
			case 'w':   letterPointer_ = DisplayLetters::letter_w_medium; width = 7; break;
			case 'x':   letterPointer_ = DisplayLetters::letter_x_medium; width = 5; break;
			case 'y':   letterPointer_ = DisplayLetters::letter_y_medium; width = 5; break;
			case 'z':   letterPointer_ = DisplayLetters::letter_z_medium; width = 5; break;

			case '0':   letterPointer_ = DisplayLetters::digit_0_medium; width = 7; break;
			case '1':   letterPointer_ = DisplayLetters::digit_1_medium; width = 7; break;
			case '2':   letterPointer_ = DisplayLetters::digit_2_medium; width = 7; break;
			case '3':   letterPointer_ = DisplayLetters::digit_3_medium; width = 7; break;
			case '4':   letterPointer_ = DisplayLetters::digit_4_medium; width = 7; break;
			case '5':   letterPointer_ = DisplayLetters::digit_5_medium; width = 7; break;
			case '6':   letterPointer_ = DisplayLetters::digit_6_medium; width = 7; break;
			case '7':   letterPointer_ = DisplayLetters::digit_7_medium; width = 7; break;
			case '8':   letterPointer_ = DisplayLetters::digit_8_medium; width = 7; break;
			case '9':   letterPointer_ = DisplayLetters::digit_9_medium; width = 7; break;

			case '%':   letterPointer_ = DisplayLetters::sign_percent_medium; 	width = 7; break;
			case '.':   letterPointer_ = DisplayLetters::sign_dot_medium; 		width = 2; break;
			case ',':   letterPointer_ = DisplayLetters::sign_komma_medium; 	width = 3; break;
			case '-':   letterPointer_ = DisplayLetters::sign_minus_medium; 	width = 4; break;
			case '#':   letterPointer_ = DisplayLetters::sign_degree_medium; 	width = 4; break;

			case '|':	width = 7; break;

			case ' ': 	width = 4; break;
			default: 	width = 4; break;
		}
		//include Letter to Display Content
		for(size_t y = 0; y < height; y++)
		{
			for(size_t x = 0; x < width; x++)
			{
				displayContent_[yCoordinate+y][xCoordinate+x] = letterPointer_[y][x];
			}
		}

	}
	else if(size == large)
	{
		size_t height = 8;

		auto* letterPointer_ = DisplayLetters::letter_space_large;
		switch(letter)
		{
			case 'A': letterPointer_ = DisplayLetters::letter_A_large; width = 9; break;
			case 'B': letterPointer_ = DisplayLetters::letter_B_large; width = 9; break;

			case ' ': 	width = 4; break;
			default: 	width = 4; break;
		}
		//include Letter to Display Content
		for(size_t y = 0; y < height; y++)
		{
			for(size_t x = 0; x < width; x++)
			{
				displayContent_[yCoordinate+y][xCoordinate+x] = letterPointer_[y][x];
			}
		}
	}

	return(width);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
size_t Display<TSpiDriver, TBackgroundColorManager>::
printDigit(uint8_t const xCoordinate, uint8_t const yCoordinate, uint8_t const digit, TextSize const size) const
{
	size_t width = 0;

	if(digit > 9)
	{
		width = printLetter(xCoordinate, yCoordinate, '|', size);
	}
	else
	{
		char digitAsLetter = (char) (digit + AsciiOffset);
		width = printLetter(xCoordinate, yCoordinate, digitAsLetter, size);
	}
	return(width);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
uint8_t Display<TSpiDriver, TBackgroundColorManager>::
getSpaceValue(TextSize const size) const
{
	switch(size)
	{
	case small: 	return(1); break;
	case medium: 	return(2); break;
	case large: 	return(2); break;
	default: 		return(1); break;
	}
}


template <typename TSpiDriver, typename TBackgroundColorManager>
bool Display<TSpiDriver, TBackgroundColorManager>::
checkBoundary(uint8_t xPosEnd, uint8_t yPosEnd) const
{
	if(xPosEnd >= xSize)
	{
		return(false);
	}
	if(yPosEnd >= ySize*2)
	{
		return(false);
	}
	return(true);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
setBGred(uint8_t const colorValue) const
{
	backgroundColor_.setRedColor(colorValue);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
setBGgreen(uint8_t const colorValue) const
{
	backgroundColor_.setGreenColor(colorValue);
}


template <typename TSpiDriver, typename TBackgroundColorManager>
void Display<TSpiDriver, TBackgroundColorManager>::
setContrast(uint8_t const contrast) const
{
	uint8_t const Cmds[] = {DSPsetContrast, (uint8_t) (2*contrast+55)};

	spiDriver_.asyncWrite(Cmds, sizeof(Cmds), DataHandling::dspCommand, nullptr);
}


} //Namespace Component

#endif
