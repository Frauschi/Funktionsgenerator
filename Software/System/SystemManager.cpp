
//-----------------------------------------------------------------------------
//------------------------------- Includes ------------------------------------
#include "SystemManager.h"
#include "SignalGenerationCommon.h"


//---------------------------------------------------------------------------------------
// ----------------------------- Method 'instance()' ------------------------------------

const System::Manager& System::instance(void)
{
	static System::Manager system;

	return system;
}


//---------------------------------------------------------------------------------------
// ------------------ Implementation of class 'System::Manager' -------------------------
System::Manager::
Manager() :
	el_(),
	coreClock_(Device::Core::systemCoreClock()),

	hardwareTimer_(TIM2, coreClock_),
	hardwareEncoder_(TIM3),

	gpioA_(GPIOA),
	gpioB_(GPIOB),
	gpioC_(GPIOC),
	gpioD_(GPIOD),

	hardwareI2C1_(),
	hardwareSPI1_(SPI1),
	hardwareSPI2_(SPI2),

	hardwarePwm1_(TIM8, GPIOC, 8),
	hardwarePwm2_(TIM8, GPIOC, 9),

	timer_(hardwareTimer_, el_),
	rotaryEncoder_(hardwareEncoder_, el_),
	backgroundColorMgr_(hardwarePwm2_, hardwarePwm1_),

	i2c1Manager_(hardwareI2C1_, el_),

	spi1Manager_(hardwareSPI1_, el_),
	spi2Manager_(hardwareSPI2_, el_),

	portExpander1IntPin_(gpioB_, Device::HardwareGpio::Pin::_7, IoPin::PinType::Input, Device::GpioIntId::portExpander1Int, el_),
	portExpander2IntPin_(gpioB_, Device::HardwareGpio::Pin::_6, IoPin::PinType::Input, Device::GpioIntId::portExpander2Int, el_),
	encoderIntPin_(gpioD_, Device::HardwareGpio::Pin::_2, IoPin::PinType::Input, Device::GpioIntId::encoderInt, el_),
	dacUpdatePin_(gpioA_, Device::HardwareGpio::Pin::_3, IoPin::PinType::Output_PushPull, Device::GpioIntId::NoInterrupt, el_),
	ddsTriggerPin_(gpioC_, Device::HardwareGpio::Pin::_2, IoPin::PinType::Output_PushPull, Device::GpioIntId::NoInterrupt, el_),
	eepromWcPin_(gpioC_, Device::HardwareGpio::Pin::_12, IoPin::PinType::Output_PushPull, Device::GpioIntId::NoInterrupt, el_),
	statusLEDPin_(gpioC_, Device::HardwareGpio::Pin::_10, IoPin::PinType::Output_PushPull, Device::GpioIntId::NoInterrupt, el_),

	displayButtons_(i2cSlaveDriver_[PortExpander2], portExpander2IntPin_, el_),
	channelButtons_(i2cSlaveDriver_[PortExpander1], portExpander1IntPin_, el_),
	encoder_(rotaryEncoder_, encoderIntPin_, el_),
	display_(spiSlaveDriver_[DSP], backgroundColorMgr_),
	eeprom_(i2cSlaveDriver_[ExternalEEPROM], eepromWcPin_, el_, timer_),
	heartbeat_(statusLEDPin_, timer_),

	frequencyController_(i2cSlaveDriver_[ClockGenerator]),
	supportVoltageGenerator_(spiSlaveDriver_[Dac], dacUpdatePin_),

	directDigitalSynthesizerCh1_(spiSlaveDriver_[DDS1], ddsTriggerPin_),
	signalGeneratorCh1_(SignalGeneration::Output::Ch1, directDigitalSynthesizerCh1_, frequencyController_, supportVoltageGenerator_),

	directDigitalSynthesizerCh2_(spiSlaveDriver_[DDS2], ddsTriggerPin_),
	signalGeneratorCh2_(SignalGeneration::Output::Ch2, directDigitalSynthesizerCh2_, frequencyController_, supportVoltageGenerator_)
{
}


System::Manager::
~Manager()
{

}

