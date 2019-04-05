
#ifndef BACKGROUNDCOLORDRIVER_H_
#define BACKGROUNDCOLORDRIVER_H_

#include <cstdint>

namespace Driver {

template <typename TPwmDevice>
class BackgroundColorDriver
{
public:

	/* Constructor */
	BackgroundColorDriver(const TPwmDevice& pwmRed, const TPwmDevice& pwmGreen);

	/* Destructor */
	~BackgroundColorDriver();

	/* Set the background color to given values
	 * Params are the percentages for red and green as values between 0 and 100 */
	void setRedColor(std::uint8_t const percentRed) const;
	void setGreenColor(std::uint8_t const percentGreen) const;

	/* Disable background color */
	void disableRedColor(void) const;
	void disableGreenColor(void) const;

private:

	const TPwmDevice& pwmRed_;
	const TPwmDevice& pwmGreen_;
};


//---------------------------------------------------------------------------------------
//----------------- Implementation of Class 'BackgroundColorDriver' ---------------------
template <typename TPwmDevice>
BackgroundColorDriver<TPwmDevice>::
BackgroundColorDriver(const TPwmDevice& pwmRed, const TPwmDevice& pwmGreen) :
	pwmRed_(pwmRed),
	pwmGreen_(pwmGreen)
{
	/* Set default values to zero */
	pwmRed_.setDutyCycle(0);
	pwmGreen_.setDutyCycle(0);

	pwmRed_.disable();
	pwmGreen_.disable();
}


template <typename TPwmDevice>
BackgroundColorDriver<TPwmDevice>::
~BackgroundColorDriver()
{
}


template <typename TPwmDevice>
void BackgroundColorDriver<TPwmDevice>::
setRedColor(std::uint8_t const percentRed) const
{
	pwmRed_.setDutyCycle(percentRed);
	pwmRed_.enable();
}

template <typename TPwmDevice>
void BackgroundColorDriver<TPwmDevice>::
setGreenColor(std::uint8_t const percentGreen) const
{
	pwmGreen_.setDutyCycle(percentGreen);
	pwmGreen_.enable();
}


template <typename TPwmDevice>
void BackgroundColorDriver<TPwmDevice>::
disableRedColor(void) const
{
	pwmRed_.disable();
}

template <typename TPwmDevice>
void BackgroundColorDriver<TPwmDevice>::
disableGreenColor(void) const
{
	pwmGreen_.disable();
}

} /* namespace Driver */

#endif /* BACKGROUNDCOLORDRIVER_H_ */
