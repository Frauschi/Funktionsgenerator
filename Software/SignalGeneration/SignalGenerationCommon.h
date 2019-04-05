
#ifndef SIGNALGENERATIONCOMMON_H
#define SIGNALGENERATIONCOMMON_H

#include <cstdint>

namespace SignalGeneration {


enum Output {
	Ch1,
	Ch2,

	NumOfOutputs
};


enum Waveform : std::uint32_t {
	Sine,
	Rect,
	Triangle,
	Saw_pos,
	Saw_neg,

	Waveformcount
};


/* Default values */
static const Waveform defaultWaveform = Waveform::Sine;
static const std::uint32_t defaultFrequency = 1000;	// 1kHz
static const std::uint32_t defaultAmplitude = 1000;	// 1V
static const std::int32_t defaultOffset = 0;
static const std::int32_t defaultPhase = 0;
static const std::uint32_t defaultDutyCycle = 50; // 50%

/* Upper and lower bounds */
static const std::uint32_t maxFrequency = 20e6; // 20MHz
static const std::uint32_t minFrequency = 1; // 1Hz
static const std::uint32_t maxAmplitude = 10000; // 10V
static const std::uint32_t minAmplitude = 100; // 100mV
static const std::int32_t maxOffset = 5000;
static const std::int32_t minOffset = -5000;
static const std::int32_t maxPhase = 180;
static const std::int32_t minPhase = -180;
static const std::uint32_t maxDutyCycle = 100;
static const std::uint32_t minDutyCycle = 0;


struct ChannelSettings{
	Waveform 		form_;
	std::uint32_t 	frequency_;
	std::uint32_t 	amplitude_;
	std::int32_t 	offset_;
	std::int32_t 	phase_;
	std::uint32_t 	dutyCycle_;

	/* Default constructor */
	ChannelSettings() :
		form_(defaultWaveform),
		frequency_(defaultFrequency),
		amplitude_(defaultAmplitude),
		offset_(defaultOffset),
		phase_(defaultPhase),
		dutyCycle_(defaultOffset)
	{}
};


}; // end namespace SignalGeneration

#endif
