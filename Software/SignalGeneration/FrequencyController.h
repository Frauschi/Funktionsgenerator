
#ifndef FREQUENCYCONTROLLER_H
#define FREQUENCYCONTROLLER_H


#include <cstdint>
#include <cmath>
#include "SignalGenerationCommon.h"


namespace SignalGeneration {


template <typename TI2cSlaveDriver>
class FrequencyController
{
public:

	/* Constructor */
	explicit FrequencyController(const TI2cSlaveDriver& i2c);

	/* Destructor */
	~FrequencyController();

	/* Initialize Si5351 clock generator */
	auto initialize(void) const -> void;

	/* Get current output frequency */
	auto getCurrentFrequency(Output const channelNo) const -> std::uint32_t;

	/* Set Si5351 PLL and divider settings to output a appropriate input frequency for the
	 * synthesizer of given channel */
	auto setFrequencyForChannel(Output const channelNo, ChannelSettings const& settings) const  -> std::uint32_t;


private:

	/* Registers of the Si5351 */
	enum : std::uint8_t {
		SI5351_REGISTER_0_DEVICE_STATUS                       = 0,
		SI5351_REGISTER_1_INTERRUPT_STATUS_STICKY             = 1,
		SI5351_REGISTER_2_INTERRUPT_STATUS_MASK               = 2,
		SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL               = 3,
		SI5351_REGISTER_9_OEB_PIN_ENABLE_CONTROL              = 9,
		SI5351_REGISTER_15_PLL_INPUT_SOURCE                   = 15,
		SI5351_REGISTER_16_CLK0_CONTROL                       = 16,
		SI5351_REGISTER_17_CLK1_CONTROL                       = 17,
		SI5351_REGISTER_18_CLK2_CONTROL                       = 18,
		SI5351_REGISTER_19_CLK3_CONTROL                       = 19,
		SI5351_REGISTER_20_CLK4_CONTROL                       = 20,
		SI5351_REGISTER_21_CLK5_CONTROL                       = 21,
		SI5351_REGISTER_22_CLK6_CONTROL                       = 22,
		SI5351_REGISTER_23_CLK7_CONTROL                       = 23,
		SI5351_REGISTER_24_CLK3_0_DISABLE_STATE               = 24,
		SI5351_REGISTER_25_CLK7_4_DISABLE_STATE               = 25,
		SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1           = 42,
		SI5351_REGISTER_43_MULTISYNTH0_PARAMETERS_2           = 43,
		SI5351_REGISTER_44_MULTISYNTH0_PARAMETERS_3           = 44,
		SI5351_REGISTER_45_MULTISYNTH0_PARAMETERS_4           = 45,
		SI5351_REGISTER_46_MULTISYNTH0_PARAMETERS_5           = 46,
		SI5351_REGISTER_47_MULTISYNTH0_PARAMETERS_6           = 47,
		SI5351_REGISTER_48_MULTISYNTH0_PARAMETERS_7           = 48,
		SI5351_REGISTER_49_MULTISYNTH0_PARAMETERS_8           = 49,
		SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1           = 50,
		SI5351_REGISTER_51_MULTISYNTH1_PARAMETERS_2           = 51,
		SI5351_REGISTER_52_MULTISYNTH1_PARAMETERS_3           = 52,
		SI5351_REGISTER_53_MULTISYNTH1_PARAMETERS_4           = 53,
		SI5351_REGISTER_54_MULTISYNTH1_PARAMETERS_5           = 54,
		SI5351_REGISTER_55_MULTISYNTH1_PARAMETERS_6           = 55,
		SI5351_REGISTER_56_MULTISYNTH1_PARAMETERS_7           = 56,
		SI5351_REGISTER_57_MULTISYNTH1_PARAMETERS_8           = 57,
		SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1           = 58,
		SI5351_REGISTER_59_MULTISYNTH2_PARAMETERS_2           = 59,
		SI5351_REGISTER_60_MULTISYNTH2_PARAMETERS_3           = 60,
		SI5351_REGISTER_61_MULTISYNTH2_PARAMETERS_4           = 61,
		SI5351_REGISTER_62_MULTISYNTH2_PARAMETERS_5           = 62,
		SI5351_REGISTER_63_MULTISYNTH2_PARAMETERS_6           = 63,
		SI5351_REGISTER_64_MULTISYNTH2_PARAMETERS_7           = 64,
		SI5351_REGISTER_65_MULTISYNTH2_PARAMETERS_8           = 65,
		SI5351_REGISTER_66_MULTISYNTH3_PARAMETERS_1           = 66,
		SI5351_REGISTER_67_MULTISYNTH3_PARAMETERS_2           = 67,
		SI5351_REGISTER_68_MULTISYNTH3_PARAMETERS_3           = 68,
		SI5351_REGISTER_69_MULTISYNTH3_PARAMETERS_4           = 69,
		SI5351_REGISTER_70_MULTISYNTH3_PARAMETERS_5           = 70,
		SI5351_REGISTER_71_MULTISYNTH3_PARAMETERS_6           = 71,
		SI5351_REGISTER_72_MULTISYNTH3_PARAMETERS_7           = 72,
		SI5351_REGISTER_73_MULTISYNTH3_PARAMETERS_8           = 73,
		SI5351_REGISTER_74_MULTISYNTH4_PARAMETERS_1           = 74,
		SI5351_REGISTER_75_MULTISYNTH4_PARAMETERS_2           = 75,
		SI5351_REGISTER_76_MULTISYNTH4_PARAMETERS_3           = 76,
		SI5351_REGISTER_77_MULTISYNTH4_PARAMETERS_4           = 77,
		SI5351_REGISTER_78_MULTISYNTH4_PARAMETERS_5           = 78,
		SI5351_REGISTER_79_MULTISYNTH4_PARAMETERS_6           = 79,
		SI5351_REGISTER_80_MULTISYNTH4_PARAMETERS_7           = 80,
		SI5351_REGISTER_81_MULTISYNTH4_PARAMETERS_8           = 81,
		SI5351_REGISTER_82_MULTISYNTH5_PARAMETERS_1           = 82,
		SI5351_REGISTER_83_MULTISYNTH5_PARAMETERS_2           = 83,
		SI5351_REGISTER_84_MULTISYNTH5_PARAMETERS_3           = 84,
		SI5351_REGISTER_85_MULTISYNTH5_PARAMETERS_4           = 85,
		SI5351_REGISTER_86_MULTISYNTH5_PARAMETERS_5           = 86,
		SI5351_REGISTER_87_MULTISYNTH5_PARAMETERS_6           = 87,
		SI5351_REGISTER_88_MULTISYNTH5_PARAMETERS_7           = 88,
		SI5351_REGISTER_89_MULTISYNTH5_PARAMETERS_8           = 89,
		SI5351_REGISTER_90_MULTISYNTH6_PARAMETERS             = 90,
		SI5351_REGISTER_91_MULTISYNTH7_PARAMETERS             = 91,
		SI5351_REGISTER_092_CLOCK_6_7_OUTPUT_DIVIDER          = 92,
		SI5351_REGISTER_165_CLK0_INITIAL_PHASE_OFFSET         = 165,
		SI5351_REGISTER_166_CLK1_INITIAL_PHASE_OFFSET         = 166,
		SI5351_REGISTER_167_CLK2_INITIAL_PHASE_OFFSET         = 167,
		SI5351_REGISTER_168_CLK3_INITIAL_PHASE_OFFSET         = 168,
		SI5351_REGISTER_169_CLK4_INITIAL_PHASE_OFFSET         = 169,
		SI5351_REGISTER_170_CLK5_INITIAL_PHASE_OFFSET         = 170,
		SI5351_REGISTER_177_PLL_RESET                         = 177,
		SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE = 183
	};

	enum Si5351PLL_t : std::uint8_t {
		SI5351_PLL_A = 0,
		SI5351_PLL_B
	};

	enum Si5351RDiv_t : std::uint8_t {
		SI5351_R_DIV_1   = 0,
		SI5351_R_DIV_2   = 1,
		SI5351_R_DIV_4   = 2,
		SI5351_R_DIV_8   = 3,
		SI5351_R_DIV_16  = 4,
		SI5351_R_DIV_32  = 5,
		SI5351_R_DIV_64  = 6,
		SI5351_R_DIV_128 = 7
	};

	enum Si5351CrystalLoad_t : std::uint16_t {
	  SI5351_CRYSTAL_LOAD_6PF  = (1<<6),
	  SI5351_CRYSTAL_LOAD_8PF  = (2<<6),
	  SI5351_CRYSTAL_LOAD_10PF = (3<<6)
	} ;

	enum Si5351CrystalFreq_t : std::uint32_t {
	  SI5351_CRYSTAL_FREQ_25MHZ = (25000000),
	  SI5351_CRYSTAL_FREQ_27MHZ = (27000000)
	};

	/* Methods to configure PLLs and multisynth dividers of the Si5351 */
	auto setPLL(Si5351PLL_t const pll, std::uint32_t const mult, std::uint32_t const num,
			std::uint32_t const denom) const -> void;

	auto setDivider(std::uint8_t const output, Si5351PLL_t const pll, std::uint32_t const msDiv,
			std::uint32_t const msNum, std::uint32_t const msDenom, Si5351RDiv_t const rDiv) const -> void;

	/* Array for the divider for each channel */
	mutable std::array<std::uint32_t, Output::NumOfOutputs> channelFrequencies_;

	const TI2cSlaveDriver& i2c_;
};


template <typename TI2cSlaveDriver>
FrequencyController<TI2cSlaveDriver>::
FrequencyController(const TI2cSlaveDriver& i2c) :
	i2c_(i2c)
{
}


template <typename TI2cSlaveDriver>
FrequencyController<TI2cSlaveDriver>::
~FrequencyController()
{
}


/* Set the multiplier for the specified PLL
 * 		pll		- PLL to configure: SI5351_PLL_A or SI5351_PLL_B
 *   	mult	- PLL integer multiplier (must be between 15 and 90)
 *   	num   	- 20-bit numerator for fractional output (0..1,048,575).
 *                 Set this to '0' for integer output.
 *   	denom 	- 20-bit denominator for fractional output (1..1,048,575).
 *                 Set this to '1' to avoid divider by zero errors.
 *
 * 	The complete divider must be between 15.0 and 36.0 (with a 25MHz XTAL)
 */
template <typename TI2cSlaveDriver>
auto FrequencyController<TI2cSlaveDriver>::
setPLL(Si5351PLL_t const pll, std::uint32_t const mult, std::uint32_t const num,
		std::uint32_t const denom) const -> void
{
	std::uint32_t P1;	/* PLL config register P1 */
	std::uint32_t P2;	/* PLL config register P2 */
	std::uint32_t P3;	/* PLL config register P3 */

	/* Feedback Multisynth Divider Equation
	 *
	 * where: a = mult, b = num and c = denom
	 *
	 * P1 register is an 18-bit value using following formula:
	 *
	 * 	P1[17:0] = 128 * mult + floor(128*(num/denom)) - 512
	 *
	 * P2 register is a 20-bit value using the following formula:
	 *
	 * 	P2[19:0] = 128 * num - denom * floor(128*(num/denom))
	 *
	 * P3 register is a 20-bit value using the following formula:
	 *
	 *  P3[19:0] = denom
	 */

	/* Set the main PLL config registers */
	if (num == 0) {
		/* Integer mode */
		P1 = (128 * mult) - 512;
		P2 = num;
		P3 = denom;
	}
	else {
		/* Fractional mode */
		P1 = static_cast<std::uint32_t>((128 * mult) +
				static_cast<std::uint32_t>(128 * (static_cast<float>(num) / static_cast<float>(denom))) - 512);

		P2 = static_cast<std::uint32_t>((128 * num) -
				(denom * static_cast<std::uint32_t>(128 * (static_cast<float>(num) / static_cast<float>(denom)))));

		P3 = denom;
	}

	/* Get the appropriate starting point for the PLL registers */
	std::uint8_t baseAddr = (pll == SI5351_PLL_A ? 26 : 34);

	/* Send the calculated PLL settings */
	std::uint8_t pllSettingsBuffer[] = {
			baseAddr, 	static_cast<std::uint8_t>((P3 & 0x0000FF00)>>8),
						static_cast<std::uint8_t>(P3 & 0x000000FF),
						static_cast<std::uint8_t>((P1 & 0x00030000)>>16),
						static_cast<std::uint8_t>((P1 & 0x0000FF00)>>8),
						static_cast<std::uint8_t>(P1 & 0x000000FF),
						static_cast<std::uint8_t>(((P3 & 0x000F0000)>>12) | ((P2 & 0x000F0000)>>16)),
						static_cast<std::uint8_t>((P2 & 0x0000FF00) >> 8),
						static_cast<std::uint8_t>(P2 & 0x000000FF)
	};
	i2c_.asyncWrite(pllSettingsBuffer, sizeof(pllSettingsBuffer), nullptr, nullptr);

	/* Reset both PLLs */
	i2c_.asyncWriteRegister(SI5351_REGISTER_177_PLL_RESET, 0x01<<7 | 0x01<<5, nullptr, nullptr);
}


/* Configures the Multisynth divider and the R_DIV dividier
 *  	output		- Output channel to use (0..2)
 *   	pllSource	- PLL input source: SI5351_PLL_A or SI5351_PLL_B
 *   	msDiv       - Integer divider for the Multisynth output: Must be between 8.0 and 1800.0
 *   					(Or exactly 4.0 for special DIVBY4 mode)
 *		msNum       - 20-bit numerator for fractional output
 *		msDenom     - 20-bit denominator for fractional output
 *		rDiv		- R_DIV divider
 */
template <typename TI2cSlaveDriver>
auto FrequencyController<TI2cSlaveDriver>::
setDivider(std::uint8_t const output, Si5351PLL_t const pll, std::uint32_t const msDiv,
		std::uint32_t const msNum, std::uint32_t const msDenom, Si5351RDiv_t const rDiv) const -> void
{
	std::uint32_t P1;	/* Multisynth config register P1 */
	std::uint32_t P2;	/* Multisynth config register P2 */
	std::uint32_t P3;    /* Multisynth config register P3 */

	/* Output Multisynth Divider Equations
	 *
	 * where: a = div, b = num and c = denom
	 *
	 * P1 register is an 18-bit value using following formula:
	 *
	 * 	P1[17:0] = 128 * a + floor(128*(b/c)) - 512
	 *
	 * P2 register is a 20-bit value using the following formula:
	 *
	 * 	P2[19:0] = 128 * b - c * floor(128*(b/c))
	 *
	 * P3 register is a 20-bit value using the following formula:
	 *
	 * 	P3[19:0] = c
	 */

	/* Set the main PLL config registers */
	if (msDiv == 4) {
		/* Div by 4 special mode */
		P1 = 0;
		P2 = 0;
		P3 = 1;
	}
	else if (msNum == 0) {
		/* Integer mode */
		P1 = (128 * msDiv) - 512;
		P2 = msNum;
		P3 = msDenom;
	}
	else {
		/* Fractional mode */
		P1 = static_cast<uint32_t>((128 * msDiv) +
				static_cast<std::uint32_t>(128 * (static_cast<float>(msNum) / static_cast<float>(msDenom))) - 512);

		P2 = static_cast<uint32_t>((128 * msNum) -
				(msDenom * static_cast<std::uint32_t>(128 * (static_cast<float>(msNum) / static_cast<float>(msDenom)))));

		P3 = msDenom;
	}

	/* Get the appropriate starting point for the multisynth registers and the appropriate ClkControl register */
	std::uint8_t baseAddr = 0, ctrlRegAddr = 0;
	switch (output) {
		case 0:
			baseAddr = SI5351_REGISTER_42_MULTISYNTH0_PARAMETERS_1;
			ctrlRegAddr = SI5351_REGISTER_16_CLK0_CONTROL;
			break;
		case 1:
			baseAddr = SI5351_REGISTER_50_MULTISYNTH1_PARAMETERS_1;
			ctrlRegAddr = SI5351_REGISTER_17_CLK1_CONTROL;
			break;
		case 2:
			baseAddr = SI5351_REGISTER_58_MULTISYNTH2_PARAMETERS_1;
			ctrlRegAddr = SI5351_REGISTER_18_CLK2_CONTROL;
			break;
	}

	/* Set the MSx config registers */
	std::uint8_t msSettingsBuffer[] = {
			baseAddr, 	static_cast<std::uint8_t>((P3 & 0x0000FF00)>>8),
						static_cast<std::uint8_t>(P3 & 0x000000FF),
						static_cast<std::uint8_t>((P1 & 0x00030000)>>16),
						static_cast<std::uint8_t>((P1 & 0x0000FF00)>>8),
						static_cast<std::uint8_t>(P1 & 0x000000FF),
						static_cast<std::uint8_t>(((P3 & 0x000F0000)>>12) | ((P2 & 0x000F0000)>>16)),
						static_cast<std::uint8_t>((P2 & 0x0000FF00) >> 8),
						static_cast<std::uint8_t>(P2 & 0x000000FF)
	};
	if (msDiv == 4) {
		/* Div by 4 special mode: Set DIVBY4 bits */
		msSettingsBuffer[3] |= 0x03<<2;
	}
	/* Set R_DIV */
	msSettingsBuffer[3] |= (rDiv & 0x07)<<4;

	/* Write settings */
	i2c_.asyncWrite(msSettingsBuffer, sizeof(msSettingsBuffer), nullptr, nullptr);

	/* Configure the clk control and enable the output */
	std::uint8_t clkControlReg = 0x0F;  /* 8mA drive strength, MS0 as CLK0 source, Clock not inverted, powered up */
	if (pll == SI5351_PLL_B) {
		clkControlReg |= 1<<5; /* Uses PLLB */
	}
	if (msNum == 0) {
		clkControlReg |= 1<<6; /* Integer mode */
	}

	i2c_.asyncWriteRegister(ctrlRegAddr, clkControlReg, nullptr, nullptr);
}


template <typename TI2cSlaveDriver>
auto FrequencyController<TI2cSlaveDriver>::
initialize(void) const -> void
{
	/* Disable all outputs setting CLKx_DIS high */
	i2c_.asyncWriteRegister(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, 0xFF, nullptr, nullptr);

	/* Power down all output drivers */
	static const std::uint8_t powerDownRegBuffer[] = {
			SI5351_REGISTER_16_CLK0_CONTROL, 0x80,
											 0x80,
						 					 0x80,
						  					 0x80,
											 0x80,
											 0x80,
											 0x80,
											 0x80
	};
	i2c_.asyncWrite(powerDownRegBuffer, sizeof(powerDownRegBuffer), nullptr, nullptr);

	/* Set the load capacitance for the XTAL */
	i2c_.asyncWriteRegister(SI5351_REGISTER_183_CRYSTAL_INTERNAL_LOAD_CAPACITANCE, SI5351_CRYSTAL_LOAD_10PF, nullptr, nullptr);

	/* Set both outputs to 180MHz */
	setPLL(SI5351_PLL_A, 28, 8, 10);
	setPLL(SI5351_PLL_B, 28, 8, 10);
	setDivider(0, SI5351_PLL_A, 4, 0, 1, SI5351_R_DIV_1);
	setDivider(1, SI5351_PLL_B, 4, 0, 1, SI5351_R_DIV_1);

	channelFrequencies_[Output::Ch1] = 180e6;
	channelFrequencies_[Output::Ch2] = 180e6;

	/* Enable outputs*/
	i2c_.asyncWriteRegister(SI5351_REGISTER_3_OUTPUT_ENABLE_CONTROL, 0x00, nullptr, nullptr);
}


template <typename TI2cSlaveDriver>
auto FrequencyController<TI2cSlaveDriver>::
getCurrentFrequency(Output const channelNo) const -> std::uint32_t
{
	return channelFrequencies_[channelNo];
}


template <typename TI2cSlaveDriver>
auto FrequencyController<TI2cSlaveDriver>::
setFrequencyForChannel(Output const channelNo, ChannelSettings const& settings) const -> std::uint32_t
{
	std::uint32_t pllMult, pllNum, pllDenom, msMult, msNum, msDenom;
	Si5351RDiv_t rDiv = SI5351_R_DIV_1;

	/* Specify which PLL and Si5351 output channel to use */
	Si5351PLL_t pll = (channelNo == Output::Ch1) ? SI5351_PLL_A : SI5351_PLL_B;
	std::uint8_t outputChannel = (channelNo == Output::Ch1) ? 0 : 1;

	/* Simplest implementation:
	 * 	Synthesizer input frequency = Target output frequency * optimal number of samples
	 *
	 * 	optimal number of samples = 720;
	 */
	static const std::uint64_t optimalNumberOfSamples 	= 720;
	static const std::uint64_t maximumInputFrequency 	= 180000000;
	static const std::uint32_t minimumInputFrequency	= 2000;

	/* Calculate input frequency */
	std::uint64_t tempValue = settings.frequency_ * optimalNumberOfSamples;
	if (tempValue < static_cast<std::uint64_t>(minimumInputFrequency)) {
		tempValue = minimumInputFrequency;
	}
	else if (tempValue > maximumInputFrequency) {
		tempValue = maximumInputFrequency;
	}
	std::uint32_t inputFrequency = static_cast<std::uint32_t>(tempValue & 0xFFFFFFFF);

	/* Store new frequency */
	channelFrequencies_[channelNo] = inputFrequency;

	/* Get proper R_div */
	if (inputFrequency >= minimumInputFrequency<<7 ) {
		rDiv = SI5351_R_DIV_1;
	}
	else if (inputFrequency < minimumInputFrequency<<1) {
		rDiv = SI5351_R_DIV_128;
	}
	else if ((inputFrequency >= minimumInputFrequency<<1) && (inputFrequency < minimumInputFrequency<<2)) {
		rDiv = SI5351_R_DIV_64;
	}
	else if ((inputFrequency >= minimumInputFrequency<<2) && (inputFrequency < minimumInputFrequency<<3)) {
		rDiv = SI5351_R_DIV_32;
	}
	else if ((inputFrequency >= minimumInputFrequency<<3) && (inputFrequency < minimumInputFrequency<<4)) {
		rDiv = SI5351_R_DIV_16;
	}
	else if ((inputFrequency >= minimumInputFrequency<<4) && (inputFrequency < minimumInputFrequency<<5)) {
		rDiv = SI5351_R_DIV_8;
	}
	else if ((inputFrequency >= minimumInputFrequency<<5) && (inputFrequency < minimumInputFrequency<<6)) {
		rDiv = SI5351_R_DIV_4;
	}
	else if ((inputFrequency >= minimumInputFrequency<<6) && (inputFrequency < minimumInputFrequency<<7)) {
		rDiv = SI5351_R_DIV_2;
	}

	/* Take R_Div into account */
	inputFrequency <<= rDiv;

	/* Setup Clock Generator */
	if (inputFrequency > 112500000) {
		/* For frequencies greater than 112.5MHz, we need the special DIVBY4 mode */
		float pllMultiplier = 4.0 * (static_cast<float>(inputFrequency) / 25000000.0);
		pllMult = static_cast<std::uint32_t>(pllMultiplier);
		pllDenom = 10000;
		pllNum = static_cast<std::uint32_t>((pllMultiplier - static_cast<float>(pllMult)) * pllDenom);

		msMult = 4;
		msNum = 0;
		msDenom = 1;
	}
	else {
		/* Set a integer PLL multiplier between 15 and 36 */
		float temp = ((static_cast<float>(inputFrequency) * 1500.0) / 25000000.0) + 0.5;
		if (temp > 36.0) {
			temp = 36.0; /* upper boundary */
		}
		pllMult = static_cast<std::uint32_t>(temp);
		pllNum = 0;
		pllDenom = 1;

		/* Calculate appropriate Multisynth divider */
		float msDiv = (25000000.0 * pllMult) / static_cast<float>(inputFrequency);
		msMult = static_cast<std::uint32_t>(msDiv);
		msDenom = 10000;
		msNum = static_cast<std::uint32_t>((msDiv - static_cast<float>(msMult)) * msDenom);
	}

	/* Apply new settings */
	setPLL(pll, pllMult, pllNum, pllDenom);
	setDivider(outputChannel, pll, msMult, msNum, msDenom, rDiv);

	return channelFrequencies_[channelNo];
}


}; /* end namespace SignalGeneration */

#endif
