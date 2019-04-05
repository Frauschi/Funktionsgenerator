
#ifndef SUPPORTVOLTAGEGENERATOR_H_
#define SUPPORTVOLTAGEGENERATOR_H_

#include <cstdint>
#include <limits>

#include "SignalGenerationCommon.h"


namespace SignalGeneration {


template <typename TSpiSlaveDriver, typename TIoPin>
class SupportVoltageGenerator
{
public:

	/* Constructor */
	SupportVoltageGenerator(const TSpiSlaveDriver& spi, const TIoPin& updatePin);

	/* Destructor */
	~SupportVoltageGenerator();

	/* Set voltage for offset addition
	 * @param voltage_mV - Signed 16bit integer twos complement
	 * 		Output voltage in tenth millivolts. Range will be automatically
	 * 		set to -25'000 <= voltage_mV <= 25'000
	 * 		Sets the voltage in a range of -2.5V <= U_out <= 2.5V */
	auto setOffsetVoltage(Output const ch, std::int16_t const voltage_mV) const -> void ;


	/* Set voltage for DDS external voltage reference
	 * @param voltage_mV - Unsigned 16bit integer
	 * 		Output voltage in tenth millivolts .Range will be automatically
	 * 		set to  0 <= voltage_mV <= 50'000
	 * 		Sets the voltage in a range of 0V <= U_out <= 5V */
	auto setAmplitudeVoltage(Output const ch, std::uint16_t const voltage_mV) const -> void;


private:

	template <typename ValueType>
	auto calculateDacValue(ValueType const inputValue) const -> ValueType;

	const TSpiSlaveDriver& spi_;
	const TIoPin& updatePin_;
};


template <typename TSpiSlaveDriver, typename TIoPin>
SupportVoltageGenerator<TSpiSlaveDriver, TIoPin>::
SupportVoltageGenerator(const TSpiSlaveDriver& spi, const TIoPin& updatePin) :
	spi_(spi),
	updatePin_(updatePin)
{
	/* Values for the zero point calibration for the outputs
	 * Number is a signed multiple of 0.125 LSB (Range: -32 LSB to +31.875 LSB) */
	std::int16_t zeroCalibrationOutput1 = 0;
	std::int16_t zeroCalibrationOutput2 = 0;


	/* Set the update pin of the multiDac to its default value */
	updatePin_.setHigh();

	/* Clear the Gain bits for all four outputs of the multiDac to set the gain = 2 */
	std::uint8_t toSend[3];
	toSend[2] = 0x00;	/* Data low byte */
	toSend[1] = 0x03;	/* Data high byte */
	toSend[0] = 0x00; 	/* Address */
	spi_.asyncWrite(toSend, 3, TSpiSlaveDriver::DataHandling::standard, nullptr);

	/* Calibrate the zero point of the two bipolar outputs */
	toSend[2] = static_cast<std::uint8_t>(zeroCalibrationOutput1 & 0xFF);	/* Data low byte */
	toSend[1] = static_cast<std::uint8_t>((zeroCalibrationOutput1 & 0x01<<15)>>15);	/* Data high byte */
	toSend[0] = 0x08; 	/* Address */
	spi_.asyncWrite(toSend, 3, TSpiSlaveDriver::DataHandling::standard, nullptr);

	toSend[2] = static_cast<std::uint8_t>(zeroCalibrationOutput2 & 0xFF);	/* Data low byte */
	toSend[1] = static_cast<std::uint8_t>((zeroCalibrationOutput2 & 0x01<<15)>>15);	/* Data high byte */
	toSend[0] = 0x09; 	/* Address */
	spi_.asyncWrite(toSend, 3, TSpiSlaveDriver::DataHandling::standard, [this]() {
		this->updatePin_.setLow();
		this->updatePin_.setHigh();
	});
}


template <typename TSpiSlaveDriver, typename TIoPin>
SupportVoltageGenerator<TSpiSlaveDriver, TIoPin>::
~SupportVoltageGenerator()
{
}


template <typename TSpiSlaveDriver, typename TIoPin>
template <typename ValueType>
auto SupportVoltageGenerator<TSpiSlaveDriver, TIoPin>::
calculateDacValue(ValueType const inputValue) const -> ValueType
{
	static constexpr ValueType upperLimit = std::numeric_limits<ValueType>::is_signed ? 25000 : 50000;
	static constexpr ValueType lowerLimit = std::numeric_limits<ValueType>::is_signed ? -upperLimit : 0;
	static constexpr float scaleFactor_ = std::numeric_limits<ValueType>::max() / static_cast<float>(upperLimit);

	ValueType retVal;

	/* Check range of input parameter and calculate 16-bit DAC value */
	if (inputValue > upperLimit) {
		retVal = static_cast<ValueType>((upperLimit * scaleFactor_) + 0.5);
	}
	else if (inputValue < lowerLimit) {
		retVal = static_cast<ValueType>((lowerLimit * scaleFactor_) + 0.5);
	}
	else {
		retVal = static_cast<ValueType>((inputValue * scaleFactor_) + 0.5);
	}

	return retVal;
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto SupportVoltageGenerator<TSpiSlaveDriver, TIoPin>::
setOffsetVoltage(Output const ch, std::int16_t const voltage_mV) const -> void
{
	/* Calculate value to set in the DAC */
	std::int16_t dacVal = calculateDacValue<std::int16_t>(voltage_mV + 275);

	/* Generate the three bytes to send */
	std::uint8_t toSend[3];
	toSend[2] = static_cast<std::uint8_t>(dacVal & 0xFF);	/* Data low byte */
	toSend[1] = static_cast<std::uint8_t>(dacVal>>8);		/* Data high byte */
	toSend[0] = (ch == Output::Ch1) ? 0x04 : 0x05; 			/* Address of DAC-0 output */

	spi_.asyncWrite(toSend, 3, TSpiSlaveDriver::DataHandling::standard, [this]() {
		this->updatePin_.setLow();
		this->updatePin_.setHigh();
	});
}


template <typename TSpiSlaveDriver, typename TIoPin>
auto SupportVoltageGenerator<TSpiSlaveDriver, TIoPin>::
setAmplitudeVoltage(Output const ch, std::uint16_t const voltage_mV) const -> void
{
	/* Calculate value to set in the DAC */
	std::uint16_t dacVal = calculateDacValue<std::uint16_t>(voltage_mV - 200);

	/* Generate the three bytes to send */
	std::uint8_t toSend[3];
	toSend[2] = static_cast<std::uint8_t>(dacVal & 0xFF);	/* Data low byte */
	toSend[1] = static_cast<std::uint8_t>(dacVal>>8);		/* Data high byte */
	toSend[0] = (ch == Output::Ch1) ? 0x06 : 0x07; 			/* Normal operation */

	spi_.asyncWrite(toSend, 3, TSpiSlaveDriver::DataHandling::standard, [this]() {
		this->updatePin_.setLow();
		this->updatePin_.setHigh();
	});
}


} /* namespace SignalGeneration */

#endif /* SUPPORTVOLTAGEGENERATOR_H_ */
