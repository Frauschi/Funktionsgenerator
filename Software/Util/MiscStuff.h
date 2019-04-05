#ifndef ERRORCODES_H
#define ERRORCODES_H

#include <cstdint>

namespace MiscStuff {


enum ErrorCode : std::uint8_t {
	Success,
	Error,

	Count
};


enum EncoderDigit : std::uint8_t {
	_firstDigit,
	_secondDigit,
	_thirdDigit,

	AmountOfDigits
};


enum EncoderCommaPosition : std::uint8_t {
	_afterFirstDigit,
	_afterSecondDigit,
	_noComma
};


enum EncoderValueSign : std::int8_t {
	_negative = -1,
	_positive = 1
};


enum EncoderMode : std::uint8_t {
	_changeDigit,
	_changePosition
};


/* Default values */
static const std::uint32_t defaultIlluminationGreen = 100; //100%
static const std::uint32_t defaultIlluminationRed = 50;	   //50%
static const std::uint32_t defaultContrast = 50;		   //50%

/* Uppper and Loewer Bounds */
static const std::uint32_t maxIlluminationGreen = 100;
static const std::uint32_t minIlluminationGreen = 0;
static const std::uint32_t maxIlluminationRed = 100;
static const std::uint32_t minIlluminationRed = 0;
static const std::uint32_t maxContrast = 70;
static const std::uint32_t minContrast = 35;

struct MainmenuSettings{
	std::uint32_t 	bgIlluminationGreen_;
	std::uint32_t 	bgIlluminationRed_;
	std::uint32_t 	contrast_;

	/* default constructor */
	MainmenuSettings() :
		bgIlluminationGreen_(defaultIlluminationGreen),
		bgIlluminationRed_(defaultIlluminationRed),
		contrast_(defaultContrast)
	{}
};


} /* end namespace MiscStuff */

#endif
