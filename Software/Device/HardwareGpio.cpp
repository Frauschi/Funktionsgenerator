
#include <HardwareGpio.h>


namespace Device {


HardwareGpio::
HardwareGpio(GPIO_TypeDef* base) :
	base_(base)
{
	/* Enable clock */
	std::uint8_t bitPos = (reinterpret_cast<std::uint32_t>(base_) - AHB2PERIPH_BASE)>>10;

	RCC->AHB2ENR	|= 0x01<<bitPos;
}

HardwareGpio::
~HardwareGpio()
{
}


} /* namespace Device */
