#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

//-----------------------------------------------------------------------------
//------------------------------- Includes ------------------------------------
// General
#include <chrono>
#include <functional>
#include "EventLoop.h"

// Devices
#include "HardwareCore.h"
#include "HardwareGpio.h"
#include "HardwareTimer.h"
#include "HardwareI2C1.h"
#include "HardwareSPI.h"
#include "HardwareEncoder.h"
#include "HardwarePWM.h"

// Driver
#include "TimerMgr.h"
#include "SpiDriver.h"
#include "IoPinDriver.h"
#include "I2cMasterDriver.h"
#include "RotaryEncoderDriver.h"
#include "BackgroundColorDriver.h"

// Components UserInterface
#include "FourButtonArray.h"
#include "Encoder.h"
#include "Display.h"
#include "ExternalEEPROM.h"
#include "SystemHeartbeat.h"

// Components SignalGeneration
#include "FrequencyController.h"
#include "SupportVoltageGenerator.h"
#include "DirectDigitalSynthesizer.h"
#include "SignalGenerator.h"


namespace System
{


//------------------------------------------------------------
//------------------------- Enums ----------------------------
enum I2C_Slave {
	PortExpander1,
	PortExpander2,
	ExternalEEPROM,
	ClockGenerator,

	I2C_count	/* Always last element! */
};


enum SPI_Slave {
	DDS1,
	DDS2,
	DSP,
	Dac,

	SPI_count
};



//------------------------------------------------------------
//------------------------ Typedefs --------------------------
// Event Loop
typedef EventLoop_t<Device::Core>	EventLoop;

// Driver
// I2C
typedef Driver::I2cMasterBusManager<Device::HardwareI2C1, EventLoop, 20> I2cMasterBusManager;
typedef Driver::I2cSlaveDriver<I2cMasterBusManager>	I2cSlaveDriver;

// Timer
typedef Driver::TimerMgr<Device::HardwareTimer, EventLoop>	Timer;
typedef Driver::RotaryEncoderDriver<Device::HardwareEncoder, EventLoop> RotaryEncoder;
typedef Driver::BackgroundColorDriver<Device::HardwarePWM> BackgroundColorManager;

//SPI
typedef Driver::SpiMasterBusManager<Device::HardwareSPI, Device::HardwareGpio, EventLoop, 20> SpiMasterBusManager;
typedef Driver::SpiSlaveDriver<SpiMasterBusManager, Device::HardwareGpio, Device::HardwareSPI::DataType> SpiSlaveDriver;

// IO Pins
typedef Driver::IoPinDriver<Device::HardwareGpio, Device::InterruptMgr, EventLoop> IoPin;

// Components UserInterface
typedef Component::FourButtonArray<I2cSlaveDriver, IoPin, EventLoop> FourButtonArray;
typedef Component::Encoder<RotaryEncoder, IoPin, EventLoop> Encoder;
typedef Component::Display<SpiSlaveDriver, BackgroundColorManager> Display;
typedef Component::ExternalEEPROM<I2cSlaveDriver, IoPin, EventLoop, Timer> EEPROM;
typedef Component::SystemHeartbeat<IoPin, Timer> Heartbeat;

// Components SignalGeneration
typedef SignalGeneration::FrequencyController<I2cSlaveDriver> FrequencyController;
typedef SignalGeneration::SupportVoltageGenerator<SpiSlaveDriver, IoPin> SupportVoltageGenerator;
typedef SignalGeneration::DirectDigitalSynthesizer<SpiSlaveDriver, IoPin> DirectDigitalSynthesizer;
typedef SignalGeneration::SignalGenerator<DirectDigitalSynthesizer, FrequencyController, SupportVoltageGenerator> SignalGenerator;


class Manager
{
public:

	//------------------------------------------------------------
	//------------------------ Methods ---------------------------
	inline const EventLoop& eventLoop(void) const;

	inline std::uint32_t coreClock(void) const;

	inline const Timer& timer(void) const;

	inline const FourButtonArray& displayButtons(void) const;
	inline const FourButtonArray& channelButtons(void) const;

	inline const Encoder& encoder(void) const;
	inline const Display& display(void) const;
	inline const EEPROM& eeprom(void) const;
	inline const Heartbeat& heartbeat(void) const;

	inline const FrequencyController& frequencyController(void) const;
	inline const SignalGenerator& signalGeneratorForChannel(SignalGeneration::Output ch) const;


	// Declare instance() method as friend to access private Constructor
	// Therefore only this method can create an instance of the Class
	friend const Manager& instance(void);


private:

	// Constructor (plus Copy and Move Ctor to prevent instanciation)
	Manager();
	Manager(const Manager&);
	Manager(Manager&&);

	// Destructor
	~Manager();

	//------------------------------------------------------------
	//------------------- System Peripherals ---------------------
	EventLoop el_;

	std::uint32_t coreClock_;

	/* Devices */
	Device::HardwareTimer hardwareTimer_;
	Device::HardwareEncoder hardwareEncoder_;

	Device::HardwareGpio gpioA_;
	Device::HardwareGpio gpioB_;
	Device::HardwareGpio gpioC_;
	Device::HardwareGpio gpioD_;

	Device::HardwareI2C1 hardwareI2C1_;
	Device::HardwareSPI hardwareSPI1_;
	Device::HardwareSPI hardwareSPI2_;

	Device::HardwarePWM hardwarePwm1_;
	Device::HardwarePWM hardwarePwm2_;

	/* Driver */
	Timer timer_;
	RotaryEncoder rotaryEncoder_;
	BackgroundColorManager backgroundColorMgr_;

	I2cMasterBusManager i2c1Manager_;

	/* Array for the slave driver
	 *	Same order as the enum I2C_Slave to get proper assignment of the slave addresses!
	 */
	std::array<I2cSlaveDriver, I2C_Slave::I2C_count> i2cSlaveDriver_ {
		I2cSlaveDriver(i2c1Manager_, 0x24),	// PortExpander 1
		I2cSlaveDriver(i2c1Manager_, 0x22), // PortExpander 2
		I2cSlaveDriver(i2c1Manager_, 0x50),	// External EEPROM
		I2cSlaveDriver(i2c1Manager_, 0x60)	// Clock Generator
	};

	SpiMasterBusManager spi1Manager_;
	SpiMasterBusManager spi2Manager_;

	std::array<SpiSlaveDriver, SPI_Slave::SPI_count> spiSlaveDriver_ {
		SpiSlaveDriver(spi1Manager_, gpioB_, Device::HardwareGpio::Pin::_0),	// DDS1
		SpiSlaveDriver(spi1Manager_, gpioC_, Device::HardwareGpio::Pin::_4),	// DDS2
		SpiSlaveDriver(spi2Manager_, gpioB_, Device::HardwareGpio::Pin::_12, gpioC_, Device::HardwareGpio::Pin::_6), // DSP
		SpiSlaveDriver(spi1Manager_, gpioB_, Device::HardwareGpio::Pin::_10),	// Dac
	};

	IoPin portExpander1IntPin_;
	IoPin portExpander2IntPin_;
	IoPin encoderIntPin_;
	IoPin dacUpdatePin_;
	IoPin ddsTriggerPin_;
	IoPin eepromWcPin_;
	IoPin statusLEDPin_;

	/* Components UserInterface */
	FourButtonArray displayButtons_;
	FourButtonArray channelButtons_;
	Encoder encoder_;
	Display display_;
	EEPROM eeprom_;
	Heartbeat heartbeat_;

	/* Components SignalGeneration */
	FrequencyController frequencyController_;
	SupportVoltageGenerator supportVoltageGenerator_;

	DirectDigitalSynthesizer directDigitalSynthesizerCh1_;
	SignalGenerator signalGeneratorCh1_;

	DirectDigitalSynthesizer directDigitalSynthesizerCh2_;
	SignalGenerator signalGeneratorCh2_;

};

// Method to get a reference to the SystemManager
const Manager& instance(void);

//---------------------------------------------------------------------------------------
// -------------------------------- Implementation --------------------------------------

inline const System::EventLoop& System::Manager::
eventLoop(void) const
{
	return el_;
}


inline std::uint32_t System::Manager::
coreClock(void) const
{
	return coreClock_;
}


inline const System::Timer& System::Manager::
timer(void) const
{
	return timer_;
}


inline const System::FourButtonArray& System::Manager::
displayButtons(void) const
{
	return displayButtons_;
}


inline const System::FourButtonArray& System::Manager::
channelButtons(void) const
{
	return channelButtons_;
}


inline const System::Encoder& System::Manager::
encoder(void) const
{
	return encoder_;
}


inline const System::Display& System::Manager::
display(void) const
{
	return display_;
}

inline const System::EEPROM& System::Manager::
eeprom(void) const
{
	return eeprom_;
}


inline const System::Heartbeat& System::Manager::
heartbeat(void) const
{
	return heartbeat_;
}

inline const System::FrequencyController& System::Manager::
frequencyController(void) const
{
	return frequencyController_;
}


inline const System::SignalGenerator& System::Manager::
signalGeneratorForChannel(SignalGeneration::Output ch) const
{
	if (ch == SignalGeneration::Output::Ch1) {
		return signalGeneratorCh1_;
	}
	else {
		return signalGeneratorCh2_;
	}
}



}	/* end namespace System */

#endif
