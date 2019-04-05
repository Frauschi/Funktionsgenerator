
#ifndef SIGNALGENERATOR_H_
#define SIGNALGENERATOR_H_


#include "SignalGenerationCommon.h"


namespace SignalGeneration {

template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
class SignalGenerator
{
public:

	/* Constructor */
	SignalGenerator(Output const outputChannel, Synthesizer const& synthesizer, FrequencyMgr const& frequencyMgr,
			VoltageHelper const& voltageHelper);

	/* Destructor */
	~SignalGenerator();

	/* Initialize all parts */
	auto initialize(ChannelSettings const& storedSettings) const -> void;

	/* Enable / Disable the signal output */
	auto setSignalOutputEnabled(bool const enable) const -> void;

	/* Modifiy channel settings. These are the callbacks used by the UserInterface classes */
	auto setWaveform(Waveform const form) const -> void;
	auto setFrequency(std::uint32_t const frequency) const -> void;
	auto setAmplitude(std::uint32_t const amplitude) const -> void;
	auto setOffset(std::int32_t const offset) const -> void;
	auto setPhase(std::int32_t const phase) const -> void;
	auto setDutyCycle(std::uint32_t const dutyCyclePercent) const -> void;


private:

	/* Store the output channel to where the generated signal is going */
	Output outputChannel_;

	/* Store current channel settings */
	mutable ChannelSettings currentSettings_;

	/* Store the current system frequency */
	mutable std::uint32_t systemFrequency_;

	/* Store the current state of the output signal */
	mutable bool outputEnabled_;

	Synthesizer const& synthesizer_;
	FrequencyMgr const& frequencyMgr_;
	VoltageHelper const& voltageHelper_;
};


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
SignalGenerator(Output const outputChannel, Synthesizer const& synthesizer, FrequencyMgr const& frequencyMgr,
		VoltageHelper const& voltageHelper) :
	outputChannel_(outputChannel),
	currentSettings_(),
	systemFrequency_(frequencyMgr.getCurrentFrequency(outputChannel_)),
	outputEnabled_(false),
	synthesizer_(synthesizer),
	frequencyMgr_(frequencyMgr),
	voltageHelper_(voltageHelper)
{
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
~SignalGenerator()
{

}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
initialize(ChannelSettings const& storedSettings) const -> void
{
	/* Store settings */
	currentSettings_ = storedSettings;

	/* Check settings to bypass invalid data read from the EEPROM */
	if ((currentSettings_.frequency_ < minFrequency) || (currentSettings_.frequency_ > maxFrequency)) {
		// 90MHz is the theoretical upper limit to get at least 2 samples
		currentSettings_.frequency_ = defaultFrequency;
	}

	if ((currentSettings_.amplitude_ < minAmplitude) || (currentSettings_.amplitude_ > maxAmplitude)) {
		currentSettings_.amplitude_ = defaultAmplitude;
	}

	if ((currentSettings_.offset_ < minOffset) || (currentSettings_.offset_ > maxOffset)) {
		currentSettings_.offset_ = defaultOffset;
	}

	if ((currentSettings_.phase_ < minPhase) || (currentSettings_.phase_ > maxPhase)) {
		currentSettings_.phase_ = defaultPhase;
	}

	if ((currentSettings_.dutyCycle_ < minDutyCycle) || (currentSettings_.dutyCycle_ > maxDutyCycle)) {
		currentSettings_.dutyCycle_ = defaultDutyCycle;
	}

	/* Store settings */
	currentSettings_ = storedSettings;

	/* Initalize sub components */
	synthesizer_.initialize();

	/* Calculate and set DDS input frequency using the FrequencyMgr */
	systemFrequency_ = frequencyMgr_.setFrequencyForChannel(outputChannel_, storedSettings);

	/* Apply stored settings. By calling setWaveform() we also update the frequency,
	 * phase and dutyCycle (if relevant) of the signal */
	setWaveform(storedSettings.form_);
	setAmplitude(storedSettings.amplitude_);
	setOffset(storedSettings.offset_);
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setSignalOutputEnabled(bool const enable) const -> void
{
	outputEnabled_ = enable;

	if (enable) {
		/* If the signal is enabled, apply correct offset */
		setOffset(currentSettings_.offset_);
	}
	else {
		/* Otherwise set offset to zero */
		voltageHelper_.setOffsetVoltage(outputChannel_, 0);
	}

	/* Start or stop the signal generation in the synthesizer */
	synthesizer_.setSignalGenerationEnabled(enable);
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setWaveform(Waveform const form) const -> void
{
	currentSettings_.form_ = form;

	synthesizer_.setOutput(currentSettings_, systemFrequency_);
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setFrequency(std::uint32_t const frequency) const -> void
{
	/* Update local data */
	currentSettings_.frequency_ = frequency;

	/* Check for invalid data */
	if ((currentSettings_.frequency_ < minFrequency) || (currentSettings_.frequency_ > maxFrequency)) {
		// 90MHz is the theoretical upper limit to get at least 2 samples
		currentSettings_.frequency_ = defaultFrequency;
	}

	/* Calculate and set an appropriate synthesizer input frequency */
	systemFrequency_ = frequencyMgr_.setFrequencyForChannel(outputChannel_, currentSettings_);

	synthesizer_.setOutput(currentSettings_, systemFrequency_);
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setAmplitude(std::uint32_t const amplitude) const -> void
{
	/* Updata local data */
	currentSettings_.amplitude_= amplitude;

	/* Check for invalid data */
	if ((currentSettings_.amplitude_ < minAmplitude) || (currentSettings_.amplitude_ > maxAmplitude)) {
		currentSettings_.amplitude_ = defaultAmplitude;
	}

	/* To set the amplitude of the output signal, we have to set the reference
	 * voltage of the DDS to a value between 0V and 5V
	 * 0V	=> 0V
	 * 5V => 10V */
	std::uint16_t refVoltage = amplitude * 5;
	voltageHelper_.setAmplitudeVoltage(outputChannel_, refVoltage);
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setOffset(std::int32_t const offset) const -> void
{
	/* Update local data */
	currentSettings_.offset_ = offset;

	/* Check for invalid data */
	if ((currentSettings_.offset_ < minOffset) || (currentSettings_.offset_ > maxOffset)) {
		currentSettings_.offset_ = defaultOffset;
	}

	/* Only set offset if the signal output is enabled */
	if (outputEnabled_) {
		/* To set the offset voltage of the output signal, we have to set the reference
		 * voltage of the voltage adder circuit to a value between -2V to +2V.
		 * This voltage is added onto the raw signal and the result is then multiplied
		 * by 5 in the final amplification stage. */
		std::int16_t refVoltage = offset * 2;

		voltageHelper_.setOffsetVoltage(outputChannel_, refVoltage);
	}
	else {
		voltageHelper_.setOffsetVoltage(outputChannel_, 0);
	}
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setPhase(std::int32_t const phase) const -> void
{
	/* Update local data */
	currentSettings_.phase_ = phase;

	/* Check for invalid data */
	if ((currentSettings_.phase_ < minPhase) || (currentSettings_.phase_ > maxPhase)) {
		currentSettings_.phase_ = defaultPhase;
	}

	/* Re-calculate synthesizer input frequency */
	//systemFrequency_ = frequencyMgr_.setFrequencyForChannel(outputChannel_, currentSettings_);

	synthesizer_.setOutput(currentSettings_, systemFrequency_);
}


template <typename Synthesizer, typename FrequencyMgr, typename VoltageHelper>
auto SignalGenerator<Synthesizer, FrequencyMgr, VoltageHelper>::
setDutyCycle(std::uint32_t const dutyCyclePercent) const -> void
{
	/* Update local data */
	currentSettings_.dutyCycle_ = dutyCyclePercent;

	/* Check for invalid data */
	if ((currentSettings_.dutyCycle_ < minDutyCycle) || (currentSettings_.dutyCycle_ > maxDutyCycle)) {
		currentSettings_.dutyCycle_ = defaultDutyCycle;
	}

	/* Re-calculate synthesizer input frequency */
	//systemFrequency_ = frequencyMgr_.setFrequencyForChannel(outputChannel_, currentSettings_);

	synthesizer_.setOutput(currentSettings_, systemFrequency_);
}


} /* namespace SignalGeneration */

#endif /* SIGNALGENERATOR_H_ */
