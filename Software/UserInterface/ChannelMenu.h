#ifndef CHANNELMENU_H_
#define CHANNELMENU_H_

#include "MenuBase.h"
#include "Submenus.h"
#include "SignalGenerationCommon.h"
#include <cstdint>
#include <stddef.h>

namespace Interface
{


class ChannelMenu : public MenuBase
{
public:
	typedef typename System::FourButtonArray::Button 		Button;
	typedef typename System::Display::TextSize 				TextSize;

	enum menuPage{page1 = 1, page2 = 2, Page_Count = 2};

	//Constructor
	ChannelMenu(const std::string& name, SignalGeneration::Output const outputChannel, const System::Manager& system, std::uint16_t const eepromBaseAdress);
	//Destructor
	~ChannelMenu(void);

	void init(void) const override;
	void enterMenu(void) const override;
	void exitMenu(void) const override;
	void showOverview(void) const override;

	bool isActivated(void) const override;
	void changeActivationTo(bool const activation) const override;
	void reloadHeader(void) const override;

	void changeMenuPageTo(menuPage const newPage) const;

private:
	const SignalGeneration::Output outputChannel_;
	mutable bool activated_;
	mutable menuPage currentPage_;

	void showSummary(void) const ;

	const ValueSubmenu<uint32_t> 	frequencyMenu_;
	const ValueSubmenu<uint32_t> 	amplitudeMenu_;
	const ValueSubmenu<int32_t> 	offsetMenu_;
	const ValueSubmenu<int32_t> 	phaseMenu_;
	const FormSubmenu				waveformMenu_;
	const ValueSubmenu<uint32_t> 	dutyCycleMenu_;

	const System::SignalGenerator& signalGenerator_;

	mutable SignalGeneration::ChannelSettings storedSettings;


}; //Class ChannelMenu


//---------------------------------------------------------------------------------------
//---------------------- Implementation of Class 'ChannelMenu' --------------------------

ChannelMenu::
ChannelMenu(const std::string& name, SignalGeneration::Output const outputChannel, const System::Manager& system, std::uint16_t const eepromBaseAdress) :
	MenuBase(name, system, eepromBaseAdress),
	outputChannel_(outputChannel),
	activated_(false),
	currentPage_(page1),
	frequencyMenu_(*this, "Frequency", "Frequency", 0x0004, SignalGeneration::defaultFrequency, 0, "Hz", SignalGeneration::minFrequency, SignalGeneration::maxFrequency),
	amplitudeMenu_(*this, "Amplitude", "Amplitude", 0x0008, SignalGeneration::defaultAmplitude, -3, "V", SignalGeneration::minAmplitude, SignalGeneration::maxAmplitude),
	offsetMenu_(*this, "Offset", "Offset", 0x000C, SignalGeneration::defaultOffset, -3, "V", SignalGeneration::minOffset, SignalGeneration::maxOffset),
	phaseMenu_(*this, "Phase",  "Phase", 0x0010, SignalGeneration::defaultPhase, 0, "#", SignalGeneration::minPhase, SignalGeneration::maxPhase),
	waveformMenu_(*this, "Waveform", "Waveform", 0x0000, SignalGeneration::defaultWaveform),
	dutyCycleMenu_(*this, "Duty Cycle", "Duty Cycle", 0x0014, SignalGeneration::defaultDutyCycle, 0, "%", SignalGeneration::minDutyCycle, SignalGeneration::maxDutyCycle),
	signalGenerator_(system.signalGeneratorForChannel(outputChannel))
{

}


ChannelMenu::
~ChannelMenu(void)
{

}


void ChannelMenu::
init(void) const
{
	/* Load last configuration from eeprom */
	if(eeprom_.isNewHardware() == false)
	{
		eeprom_.loadMenuValues(eepromBaseAdress_, &storedSettings, [this](){

			/* Initialize signalGenerator component */
			signalGenerator_.initialize(storedSettings);

			/* Write last configuration into the submenus */
			frequencyMenu_.setBootValue(storedSettings.frequency_);
			amplitudeMenu_.setBootValue(storedSettings.amplitude_);
			offsetMenu_.setBootValue(storedSettings.offset_);
			phaseMenu_.setBootValue(storedSettings.phase_);
			dutyCycleMenu_.setBootValue(storedSettings.dutyCycle_);
			waveformMenu_.setBootForm(storedSettings.form_);
		});
	}
	else //if EEPROM is new Hardware, save Reset Values
	{
		eeprom_.saveMenuValues(eepromBaseAdress_, &storedSettings, nullptr);
		/* Initialize signalGenerator component */
		signalGenerator_.initialize(storedSettings);

		/* Write last configuration into the submenus */
		frequencyMenu_.setBootValue(storedSettings.frequency_);
		amplitudeMenu_.setBootValue(storedSettings.amplitude_);
		offsetMenu_.setBootValue(storedSettings.offset_);
		phaseMenu_.setBootValue(storedSettings.phase_);
		dutyCycleMenu_.setBootValue(storedSettings.dutyCycle_);
		waveformMenu_.setBootForm(storedSettings.form_);
	}

	/* Set callbacks to update values */
	frequencyMenu_.setUpdateValueCallback([&](uint32_t value) {
		signalGenerator_.setFrequency(value);
	});

	amplitudeMenu_.setUpdateValueCallback([&](uint32_t value) {
		signalGenerator_.setAmplitude(value);
	});

	offsetMenu_.setUpdateValueCallback([&](int32_t value) {
		signalGenerator_.setOffset(value);
	});

	phaseMenu_.setUpdateValueCallback([&](int32_t value) {
		signalGenerator_.setPhase(value);
	});

	waveformMenu_.setUpdateWaveformCallback([&](SignalGeneration::Waveform form) {
		signalGenerator_.setWaveform(form);
	});

	dutyCycleMenu_.setUpdateValueCallback([&](uint32_t value) {
		signalGenerator_.setDutyCycle(value);
	});

}


void ChannelMenu::
enterMenu(void)const
{
	currentSubMenu_ = nullptr;

	display_.reloadHeaderContent(name_, activated_, currentPage_, menuPage::Page_Count);

	if(currentPage_ == page1)
	{
		display_.reloadButtonContent(	frequencyMenu_.getButtonName(),
										amplitudeMenu_.getButtonName(),
										offsetMenu_.getButtonName(),
										"Page 2" );
		displayButtons_.addHandlerForButton(Button::_1, [&]() {
			currentSubMenu_ = &frequencyMenu_;
			frequencyMenu_.enterSubmenu();
		});
		displayButtons_.addHandlerForButton(Button::_2, [&]() {
			currentSubMenu_ = &amplitudeMenu_;
			amplitudeMenu_.enterSubmenu();
		});
		displayButtons_.addHandlerForButton(Button::_3, [&]() {
			currentSubMenu_ = &offsetMenu_;
			offsetMenu_.enterSubmenu();
		});
		displayButtons_.addHandlerForButton(Button::_4, [&]() {
			changeMenuPageTo(page2);
		});
	}
	else if(currentPage_ == page2)
	{
		if(waveformMenu_.getWaveform() == SignalGeneration::Waveform::Rect) {
			display_.reloadButtonContent(	"Page 1",
											phaseMenu_.getButtonName(),
											waveformMenu_.getButtonName(),
											dutyCycleMenu_.getButtonName());
		}
		else {
			display_.reloadButtonContent(	"Page 1",
											phaseMenu_.getButtonName(),
											waveformMenu_.getButtonName(),
											" ");
		}
		displayButtons_.addHandlerForButton(Button::_1, [&]() {
			changeMenuPageTo(page1);
		});
		displayButtons_.addHandlerForButton(Button::_2, [&]() {
			currentSubMenu_ = &phaseMenu_;
			phaseMenu_.enterSubmenu();
		});
		displayButtons_.addHandlerForButton(Button::_3, [&]() {
			currentSubMenu_ = &waveformMenu_;
			waveformMenu_.enterSubmenu();
		});
		if(waveformMenu_.getWaveform() == SignalGeneration::Waveform::Rect) {
			displayButtons_.addHandlerForButton(Button::_4, [&]() {
				currentSubMenu_ = &dutyCycleMenu_;
				dutyCycleMenu_.enterSubmenu();
			});
		}
		else {
			displayButtons_.addHandlerForButton(Button::_4, []() { });
		}
	}

	showOverview();

	display_.drawMenuBorder();
	display_.reloadContent();
}


void ChannelMenu::
exitMenu(void) const
{
	if(currentSubMenu_ != nullptr)
	{
		currentSubMenu_->exitSubmenu();
		currentSubMenu_ = nullptr;
	}
	display_.clearContent();
}


void ChannelMenu::
showOverview(void) const
{
	frequencyMenu_.printSummary(10, headerHeight + 8);
	amplitudeMenu_.printSummary(10, headerHeight + 8 + yMenuNameShift);
	offsetMenu_.printSummary(10, headerHeight + 8 + 2*yMenuNameShift);
	phaseMenu_.printSummary(10, headerHeight + 8 + 3*yMenuNameShift);
	waveformMenu_.printSummary(10, headerHeight + 8 + 4*yMenuNameShift);

	if(waveformMenu_.getWaveform() == SignalGeneration::Waveform::Rect)
	{
		dutyCycleMenu_.printSummary(10, headerHeight + 8 + 5*yMenuNameShift);
	}
}


bool ChannelMenu::
isActivated(void) const
{
	return(activated_);
}


void ChannelMenu::
changeActivationTo(bool activation) const
{
	activated_ = activation;

	signalGenerator_.setSignalOutputEnabled(activation);
}


void ChannelMenu::
reloadHeader(void) const
{
	display_.reloadHeaderContent(name_, activated_, currentPage_, menuPage::Page_Count, true);
}


void ChannelMenu::
changeMenuPageTo(menuPage const newPage) const
{
	currentPage_ = newPage;
	display_.clearContent();
	enterMenu();
}


} //namespace Interface

#endif //CHANNELMENU_H_
