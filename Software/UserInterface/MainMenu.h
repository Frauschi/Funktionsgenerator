
#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "MenuBase.h"
#include "Submenus.h"
#include <cstdint>
#include <stddef.h>

namespace Interface
{


class MainMenu : public MenuBase
{
public:
	typedef typename System::FourButtonArray::Button 		Button;
	typedef typename System::Display::TextSize 				TextSize;

	//Constructor
	MainMenu(const std::string& name, const System::Manager& system, std::uint16_t const eepromBaseAdress);
	//Destructor
	~MainMenu(void);

	void init(void) const override;
	void enterMenu(void) const override;
	void exitMenu(void) const override;
	void showOverview(void) const override;

private:
	const ValueSubmenu<uint32_t> illuminationGreenMenu_;
	const ValueSubmenu<uint32_t> illuminationRedMenu_;
	const ValueSubmenu<uint32_t> contrastMenu_;
	const CreditsSubmenu creditsMenu_;

	mutable MiscStuff::MainmenuSettings storedSettings;

}; //Class MainMenu

//---------------------------------------------------------------------------------------
//----------------------- Implementation of Class 'MainMenu' ----------------------------

MainMenu::
MainMenu(const std::string& name, const System::Manager& system, std::uint16_t const eepromBaseAdress) :
	MenuBase(name, system, eepromBaseAdress),
	illuminationGreenMenu_(*this, "BG Illumination  Green", "Green BG", 0x0000, MiscStuff::defaultIlluminationGreen, 0, "%", MiscStuff::minIlluminationGreen, MiscStuff::maxIlluminationGreen),
	illuminationRedMenu_(*this, "BG Illumination Red", "Red BG", 0x0004, MiscStuff::defaultIlluminationRed, 0, "%", MiscStuff::minIlluminationRed, MiscStuff::maxIlluminationRed),
	contrastMenu_(*this, "Contrast", "Contrast", 0x0008, MiscStuff::defaultContrast, 0, "%", MiscStuff::minContrast, MiscStuff::maxContrast),
	creditsMenu_(*this, "Credits", "Credits", 0x000C)
{

}


MainMenu::
~MainMenu(void)
{

}


void MainMenu::
init(void) const
{
	/* Load last configuration from eeprom */
	if(eeprom_.isNewHardware() == false)
		{
			eeprom_.loadMenuValues(eepromBaseAdress_, &storedSettings, [this](){
				/* Write last configuration into the submenus */
				illuminationGreenMenu_.setBootValue(storedSettings.bgIlluminationGreen_);
				illuminationRedMenu_.setBootValue(storedSettings.bgIlluminationRed_);
				contrastMenu_.setBootValue(storedSettings.contrast_);
			});
		}
		else //if EEPROM is new Hardware, save Reset Values
		{
			eeprom_.saveMenuValues(eepromBaseAdress_, &storedSettings, [](){});
			/* Write last configuration into the submenus */
			illuminationGreenMenu_.setBootValue(storedSettings.bgIlluminationGreen_);
			illuminationRedMenu_.setBootValue(storedSettings.bgIlluminationRed_);
			contrastMenu_.setBootValue(storedSettings.contrast_);
		}

	illuminationGreenMenu_.setUpdateValueCallback([this](uint32_t const value){
		display_.setBGgreen(static_cast<uint8_t>(value));
	});

	illuminationRedMenu_.setUpdateValueCallback([this](uint32_t const value){
		display_.setBGred(static_cast<uint8_t>(value));
	});

	contrastMenu_.setUpdateValueCallback([this](uint32_t const value){
		display_.setContrast(static_cast<uint8_t>(value));
	});

}


void MainMenu::
enterMenu(void) const
{
	currentSubMenu_ = nullptr;

	//set User Settings after Bootscreen
	illuminationGreenMenu_.callUpdateValueFunction();
	illuminationRedMenu_.callUpdateValueFunction();
	contrastMenu_.callUpdateValueFunction();

	display_.reloadButtonContent(	illuminationGreenMenu_.getButtonName(),
									illuminationRedMenu_.getButtonName(),
									contrastMenu_.getButtonName(),
									creditsMenu_.getButtonName());
	display_.reloadHeaderContent(name_);

	displayButtons_.addHandlerForButton(Button::_1, [&]() {
		currentSubMenu_ = &illuminationGreenMenu_;
		illuminationGreenMenu_.enterSubmenu();
	});
	displayButtons_.addHandlerForButton(Button::_2, [&]() {
		currentSubMenu_ = &illuminationRedMenu_;
		illuminationRedMenu_.enterSubmenu();
	});
	displayButtons_.addHandlerForButton(Button::_3, [&]() {
		currentSubMenu_ = &contrastMenu_;
		contrastMenu_.enterSubmenu();
	});
	displayButtons_.addHandlerForButton(Button::_4, [&]() {
		currentSubMenu_ = &creditsMenu_;
		creditsMenu_.enterSubmenu();
	});

	showOverview();

	display_.drawMenuBorder();
	display_.reloadContent();
}


void MainMenu::
exitMenu(void) const
{
	if(currentSubMenu_ != nullptr)
	{
		currentSubMenu_->exitSubmenu();
		currentSubMenu_ = nullptr;
	}
	display_.clearContent();
}


void MainMenu::
showOverview(void) const
{
	creditsMenu_.printSummary(10, headerHeight + 8);

	illuminationGreenMenu_.printSummary(10, headerHeight + 8 + 3*yMenuNameShift);
	illuminationRedMenu_.printSummary(10, headerHeight + 8 + 4*yMenuNameShift);
	contrastMenu_.printSummary(10, headerHeight + 8 + 5*yMenuNameShift);
}

} //namespace Interface

#endif //MAINMENU_H_
