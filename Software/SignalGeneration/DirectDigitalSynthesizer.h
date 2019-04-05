
#ifndef DIRECTDIGITALSYNTHESIZER_H_
#define DIRECTDIGITALSYNTHESIZER_H_

#include <cstdint>
#include <limits>

#include "SignalGenerationCommon.h"


namespace SignalGeneration {


template <typename TSpiSlaveDriver, typename TIoPin>
class DirectDigitalSynthesizer
{
public:

	/* Constructor */
	DirectDigitalSynthesizer(const TSpiSlaveDriver& spi, const TIoPin& tiggerPin);

	/* Destructor */
	~DirectDigitalSynthesizer();

	/* Initialization */
	auto initialize(void) const -> void;

	/* Set output according to given settings */
	auto setOutput(ChannelSettings const& newSettings, std::uint32_t inputFrequency) const -> void;

	/* Enable / Disable signal generation on output */
	auto setSignalGenerationEnabled(bool const enabled) const -> void;

private:

	enum Register : std::uint16_t {
		SPICONFIG = 0x00,
		POWERCONFIG,
		CLOCKCONFIG,
		REFADJ,
		DACGAIN = 0x07,
		DACRANGE,
		DACRSET = 0x0C,
		CALCONFIG,
		COMPOFFSET,
		RAMUPDATE = 0x1D,
		PAT_STATUS,
		PAT_TYPE,
		PATTERN_DLY,
		DACDOF = 0x25,
		WAV_CONFIG = 0x27,
		PAT_TIMEBASE,
		PAT_PERIOD,
		DAC_PAT = 0x2B,
		DOUT_START,
		DOUT_CONFIG,
		DAC_CST = 0x31,
		DAC_DGAIN = 0x35,
		SAW_CONFIG = 0x37,
		DDS_TW32 = 0x3E,
		DDS_TW1,
		DDS_PW = 0x43,
		TRIG_TW_SEL,
		DDS_CONFIG,
		TW_RAM_CONFIG = 0x47,
		START_DELAY = 0x5C,
		START_ADDR,
		STOP_ADDR,
		DDS_CYC,
		CFG_ERROR,
		SRAM_DATA = 0x6000
	};

	struct CustomWaveform {
		std::uint8_t* 	buffer;
		std::uint16_t 	numOfSamples;
		std::uint16_t	startAddr;
		std::uint16_t	stopAddr;
	};

	template <typename TFunc>
	auto writeRegister(std::uint16_t const address, std::uint16_t const data, TFunc&& callback) const -> void;

	template <typename TFunc>
	auto writeSramData(std::uint8_t* const data, std::size_t const numOfBytes, TFunc&& callback) const -> void;

	/* Set the tuning word for the 24-bit frequency divider */
	auto setTuningWord(std::uint32_t const inputFrequency, std::uint32_t const targetFrequency) const -> void;

	auto createRamp(Waveform const type, std::uint32_t const inputFreq,
			std::uint32_t const targetFreq, std::int32_t const phase) const -> void;

	auto createTriangle(std::uint32_t const inputFreq, std::uint32_t const targetFreq,
			std::int32_t const phase) const -> void;

	auto createPwm(std::uint32_t const inputFreq, std::uint32_t const targetFreq,
			std::int32_t const phase, std::uint32_t const dutyCycle) const -> void;

	mutable bool outputEnabled_;
	mutable CustomWaveform sramWave_;

	const TSpiSlaveDriver& spi_;
	const TIoPin& triggerPin_;
};



template <typename TSpiSlaveDriver, typename TIoPin>
DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
DirectDigitalSynthesizer(const TSpiSlaveDriver& spi, const TIoPin& tiggerPin) :
	outputEnabled_(false),
	sramWave_({nullptr, 0, 0, 0}),
	spi_(spi),
	triggerPin_(tiggerPin)
{
	/* Set trigger pin to high to disable signal generation */
	triggerPin_.setHigh();

	/* Allocate memory for the samples buffer.
	 * Each sample value occupies 4 bytes in memory (two address bytes and two data bytes) */
	sramWave_.buffer = reinterpret_cast<std::uint8_t*>(malloc(4 * 4096));
}


template <typename TSpiSlaveDriver, typename TIoPin>
DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
~DirectDigitalSynthesizer()
{
	/* Free memory of samples buffer */
	if (sramWave_.buffer) {
		free(sramWave_.buffer);
	}
}


template <typename TSpiSlaveDriver, typename TIoPin>
template <typename TFunc>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
writeRegister(std::uint16_t address, std::uint16_t data, TFunc&& callback) const -> void
{
	std::uint32_t toSend = __REV(address<<16 | data);

	spi_.asyncWrite(reinterpret_cast<std::uint8_t*>(&toSend), 4, TSpiSlaveDriver::DataHandling::ddsCommand, callback);
}


template <typename TSpiSlaveDriver, typename TIoPin>
template <typename TFunc>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
writeSramData(std::uint8_t* data, std::size_t numOfBytes, TFunc&& callback) const -> void
{
	spi_.asyncWrite(data, numOfBytes, TSpiSlaveDriver::DataHandling::ddsData, callback);
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
setTuningWord(std::uint32_t const inputFrequency, std::uint32_t const targetFrequency) const -> void
{
	/* Calculate divider to be sent to the DDS */
	float dividerFactor = static_cast<float>(inputFrequency) / static_cast<float>(0xFFFFFF);
	std::uint32_t frequencyDiv = (targetFrequency / dividerFactor) + 0.5;

	/* Write new divider into the DDS registers */
	writeRegister(DDS_TW32, (frequencyDiv & 0xFFFF00)>>8, nullptr);
	writeRegister(DDS_TW1, (frequencyDiv & 0xFF)<<8, nullptr);

}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
createRamp(Waveform const type, std::uint32_t const inputFreq,
		std::uint32_t const targetFreq, std::int32_t const phase) const -> void
{
    /* Allocate used variables */
    std::uint32_t* tempPtr = reinterpret_cast<std::uint32_t*>(sramWave_.buffer);
	std::uint16_t address = SRAM_DATA;
	float value = 0;

	/* Calculate the number of samples */
	std::uint16_t numOfSamples = static_cast<std::uint16_t>((static_cast<float>(inputFreq) / targetFreq) + 0.5);

	/* Calculate the y delta between two samples */
	float amplitudeDelta = (std::numeric_limits<std::int16_t>::max() - std::numeric_limits<std::int16_t>::min())
			/ static_cast<float>(numOfSamples - 1);

	/* Calculate the shift in the samples to represent the phase */
	float numOfShiftSamples = (static_cast<float>(phase) / 360.0) * numOfSamples;

	/* Calculate first sample value */
	if (type == Waveform::Saw_pos) {
		value = std::numeric_limits<std::int16_t>::min() + (numOfShiftSamples * amplitudeDelta);
	}
	else if (type == Waveform::Saw_neg) {
		value = std::numeric_limits<std::int16_t>::max() - (numOfShiftSamples * amplitudeDelta);
	}

    for (std::size_t i = 0; i < numOfSamples; i++) {
    	/* Build the 4 byte reprensenting the address and the value of the sample */
    	std::int16_t valueInt = 0;
    	if (value >= 0) {
    		valueInt = static_cast<std::int16_t>(value + 0.5);
    	}
    	else {
    		valueInt = static_cast<std::int16_t>(value - 0.5);
    	}
    	std::uint32_t rawData = (address << 16) | static_cast<std::uint16_t>(valueInt);

    	/* Store current sample in the buffer.
    	 * Reverse byte order to be compatible with the little endia architecture */
		*tempPtr = __REV(rawData);

		/* Increase address by 1 and tempPtr by 4 */
		address++;
		tempPtr++;

		/* Calculate next sample */
		if (type == Waveform::Saw_pos) {
			value += amplitudeDelta;
		}
		else if (type == Waveform::Saw_neg) {
			value -= amplitudeDelta;
		}
    }

    /* Store custum wave parameters */
    sramWave_.numOfSamples = numOfSamples;
    sramWave_.startAddr = 0;
    sramWave_.stopAddr = numOfSamples - 1;
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
createTriangle(std::uint32_t const inputFreq, std::uint32_t const targetFreq,
		std::int32_t const phase) const -> void
{
	/* Allocate used variables */
	std::uint32_t* tempPtr = reinterpret_cast<std::uint32_t*>(sramWave_.buffer);
	float value = 0;
	bool signalRising = true; /* Direction of the signal: rising or falling */
	std::uint16_t address = SRAM_DATA;

	/* Calculate the number of samples and the half of that */
	std::uint16_t numOfSamples = static_cast<std::uint16_t>((static_cast<float>(inputFreq) / targetFreq) + 0.5);
	std::uint16_t halfOfSamples = (numOfSamples + 1) / 2;

	/* Calculate the y delta between two samples */
	float amplitudeDelta = (std::numeric_limits<std::int16_t>::max() - std::numeric_limits<std::int16_t>::min())
			/ static_cast<float>(halfOfSamples);

	/* Calculate the the shift in the samples to represent the given phase */
	std::uint16_t numOfShiftSamples;
	if (phase >= 0) {
		numOfShiftSamples = static_cast<std::uint16_t>(((static_cast<float>(phase) / 360.0) * numOfSamples) + 0.5);
	}
	else {
		numOfShiftSamples = static_cast<std::uint16_t>((numOfSamples * (1 + (static_cast<float>(phase) / 360.0))) + 0.5);
	}

	/* Calculate the first sample and the direction of the signal based on the calculated shift */
	if (numOfShiftSamples >= halfOfSamples) {
		numOfShiftSamples -= halfOfSamples; // Update the shift due to the updated signal direction
		value = std::numeric_limits<std::int16_t>::max() - (numOfShiftSamples * amplitudeDelta);
		signalRising = false;
	}
	else {
		value = std::numeric_limits<std::int16_t>::min() + (numOfShiftSamples * amplitudeDelta);
	}

	/* Calculate the remaining samples */
	std::uint16_t remainingSamples = halfOfSamples - numOfShiftSamples;

	for (std::size_t i = 0; i < numOfSamples; i++) {
		/* Build the 4 byte reprensenting the address and the value of the sample */
		std::int16_t valueInt = 0;
		if (value >= 0) {
			valueInt = static_cast<std::int16_t>(value + 0.5);
		}
		else {
			valueInt = static_cast<std::int16_t>(value - 0.5);
		}
		std::uint32_t rawData = (address << 16) | static_cast<std::uint16_t>(valueInt);

		/* Store current sample in the buffer.
		 * Reverse byte order to be compatible with the little endia architecture */
		*tempPtr = __REV(rawData);

		/* Increase address by 1 and tempPtr by 4 */
		address++;
		tempPtr++;

		/* Calculate next sample */
		if (remainingSamples > 0) {
			if (signalRising) {
				value += amplitudeDelta;
			}
			else {
				value -= amplitudeDelta;
			}
			remainingSamples -= 1;
			if (remainingSamples == 0) {
				signalRising = not signalRising;
			}
		}
		else if (halfOfSamples > 0) {
			if (signalRising) {
				value += amplitudeDelta;
			}
			else {
				value -= amplitudeDelta;
			}
			halfOfSamples -= 1;
			if (halfOfSamples == 0) {
				signalRising = not signalRising;
			}
		}
		else {
			if (signalRising) {
				value += amplitudeDelta;
			}
			else {
				value -= amplitudeDelta;
			}
		}
	}

	/* Store custum wave parameters */
	sramWave_.numOfSamples = numOfSamples;
	sramWave_.startAddr = 0;
	sramWave_.stopAddr = numOfSamples - 1;
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
createPwm(std::uint32_t const inputFreq, std::uint32_t const targetFreq,
		std::int32_t const phase, std::uint32_t const dutyCycle) const -> void
{
	/* Allocate used variables */
	std::uint32_t* tempPtr = reinterpret_cast<std::uint32_t*>(sramWave_.buffer);
	std::uint16_t address = SRAM_DATA;
	std::int16_t value;

	/* Calculate the number of samples */
	std::uint16_t numOfSamples = static_cast<std::uint16_t>((static_cast<float>(inputFreq) / targetFreq) + 0.5);

	/* Calculate the shift in the samples to represent the phase */
	std::uint16_t numOfShiftSamples = 0;
	if (phase >= 0) {
		numOfShiftSamples = static_cast<std::uint16_t>(((static_cast<float>(phase) / 360.0) * numOfSamples) + 0.5);
	}
	else {
		numOfShiftSamples = static_cast<std::uint16_t>((numOfSamples * (1 + (static_cast<float>(phase) / 360.0))) + 0.5);
	}

	/* Calculate the number of high samples */
	std::uint16_t numOfHighSamples = static_cast<std::uint16_t>(((static_cast<float>(numOfSamples) * dutyCycle) / 100.0) + 0.5);

	/* Calculate the number of high samples before the phase shift starts */
	std::int16_t numOfHighsBeforeShift = (numOfHighSamples + numOfShiftSamples) - numOfSamples;

	for (std::size_t i = 0; i < numOfSamples; i++) {
		/* Check if we still have to shift to get the phase right */
		if (numOfShiftSamples > 0) {
			if (numOfHighsBeforeShift > 0) {
				value = std::numeric_limits<std::int16_t>::max();
				numOfHighsBeforeShift -= 1;
			}
			else {
				value = std::numeric_limits<std::int16_t>::min();
			}
			numOfShiftSamples -= 1;
		}
		else if (numOfHighSamples > 0) {
			value = std::numeric_limits<std::int16_t>::max();
			numOfHighSamples -= 1;
		}
		else {
			value = std::numeric_limits<std::int16_t>::min();
		}

		/* Build the 4 byte reprensenting the address and the value of the sample */
		std::uint32_t rawData = (address << 16) | static_cast<std::uint16_t>(value);

		/* Store current sample in the buffer.
		 * Reverse byte order to be compatible with the little endian architecture */
		*tempPtr = __REV(rawData);

		/* Increase address by 1 and tempPtr by 4 */
		address++;
		tempPtr++;
	}

	/* Store custum wave parameters */
	sramWave_.numOfSamples = numOfSamples;
	sramWave_.startAddr = 0;
	sramWave_.stopAddr = numOfSamples - 1;
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
initialize(void) const -> void
{
	/* Software reset of all registers */
	writeRegister(SPICONFIG, 0x2004, nullptr);

	/* Set default SPI settings */
	writeRegister(SPICONFIG, 0x00, nullptr);

	/* Disable internal voltage reference and use the external */
	writeRegister(POWERCONFIG, 0x01<<4, nullptr);

	/* Set digital gain to +1 */
	writeRegister(DAC_DGAIN, 0x4000, nullptr);

	/* Update settings */
	writeRegister(RAMUPDATE, 0x01, nullptr);

	/* Pattern run continuously */
	writeRegister(PAT_TYPE, 0x00, nullptr);

	/* No digital offset onto the DAC samples */
	writeRegister(DACDOF, 0x0000, nullptr);

	/* Timing for SRAM sample reading */
	writeRegister(PAT_TIMEBASE, 0x0111, nullptr);

	/* Default delay between falling edge on trigger and begin of pattern generation */
	writeRegister(PATTERN_DLY, 0x00E, nullptr);

	/* Trigger delay is for all patterns */
	writeRegister(TRIG_TW_SEL, 0x00, nullptr);

	/* Update settings */
	writeRegister(RAMUPDATE, 0x01, nullptr);
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
setOutput(ChannelSettings const& newSettings, std::uint32_t inputFrequency) const -> void
{
	/* Store current status of trigger pin */
	bool triggerCurrentlyLow = triggerPin_.isLow();

	if (outputEnabled_) {
		/* Disable pattern generation */
		triggerPin_.setHigh();
		writeRegister(PAT_STATUS, 0x00, nullptr);
		writeRegister(RAMUPDATE, 0x01, nullptr);
	}
	else if (triggerCurrentlyLow) {
		triggerPin_.setHigh();
	}

	if (newSettings.form_ == Waveform::Sine) {
		/* Set output to prestored waveform from DDS */
		writeRegister(WAV_CONFIG, 0x01 | 0x03<<4, nullptr);

		/* Set frequency */
		setTuningWord(inputFrequency, newSettings.frequency_);

		/* Set phase */
		static constexpr float phaseScaleFactor = std::numeric_limits<std::uint16_t>::max() / 360.0;
		std::uint16_t phaseWord = static_cast<std::uint16_t>((newSettings.phase_ + 180) * phaseScaleFactor);
		writeRegister(DDS_PW, phaseWord, nullptr);

		/* Update settings */
		writeRegister(RAMUPDATE, 0x01, nullptr);
	}
	else {
		/* Create samples for waveform and configure registers */
		if (newSettings.form_ == Waveform::Saw_pos) {
			createRamp(Waveform::Saw_pos, inputFrequency, newSettings.frequency_, newSettings.phase_);
		}
		else if (newSettings.form_ == Waveform::Saw_neg) {
			createRamp(Waveform::Saw_neg, inputFrequency, newSettings.frequency_, newSettings.phase_);
		}
		else if (newSettings.form_ == Waveform::Triangle) {
			createTriangle(inputFrequency, newSettings.frequency_, newSettings.phase_);
		}
		else if (newSettings.form_ == Waveform::Rect) {
			createPwm(inputFrequency, newSettings.frequency_, newSettings.phase_, newSettings.dutyCycle_);
		}

		/* Set output to SRAM data */
		writeRegister(WAV_CONFIG, 0x00, nullptr);

		/* Set pattern duration */
		writeRegister(PAT_PERIOD, sramWave_.numOfSamples, nullptr);

		/* Update settings */
		writeRegister(RAMUPDATE, 0x01, nullptr);

		/* Set start and stop address in SRAM */
		writeRegister(START_ADDR, sramWave_.startAddr<<4, nullptr);
		writeRegister(STOP_ADDR, sramWave_.stopAddr<<4, nullptr);

		/* Enalbe SRAM write access */
		writeRegister(PAT_STATUS, 0x01<<2, nullptr);

		/* Update settings */
		writeRegister(RAMUPDATE, 0x01, nullptr);

		/* Write samples to SRAM */
		writeSramData(sramWave_.buffer, sramWave_.numOfSamples * 4, nullptr);

		/* Disable SRAM write access */
		writeRegister(PAT_STATUS, 0x00, nullptr);

		/* Update settings */
		writeRegister(RAMUPDATE, 0x01, nullptr);
	}

	if (outputEnabled_) {
		/* Enable pattern generation */
		writeRegister(PAT_STATUS, 0x01, [&]() {
			triggerPin_.setLow();
		});
	}
	else if (triggerCurrentlyLow) {
		triggerPin_.setLow();
	}
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto DirectDigitalSynthesizer<TSpiSlaveDriver, TIoPin>::
setSignalGenerationEnabled(bool enabled) const -> void
{
	outputEnabled_ = enabled;

	/* Store current status of trigger pin */
	bool triggerCurrentlyLow = triggerPin_.isLow();

	if (enabled) {
		if (triggerCurrentlyLow) {
			/* Pull trigger pin high first to create a falling edge */
			triggerPin_.setHigh();
		}

		/* Enable pattern generation using the Trigger pin */
		writeRegister(PAT_STATUS, 0x01, [&]() {
			triggerPin_.setLow();
		});
	}
	else {
		/* Disable pattern generation */
		triggerPin_.setHigh();
		writeRegister(PAT_STATUS, 0x00, nullptr);
		writeRegister(RAMUPDATE, 0x01, [this, triggerCurrentlyLow](){
			/* Set pin to low to start pattern generation on the other channel */
			if (triggerCurrentlyLow)
				this->triggerPin_.setLow();
		});
	}
}


} /* namespace SignalGeneration */

#endif /* DIRECTDIGITALSYNTHESIZER_H_ */
