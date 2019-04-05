/*
 * BootscreenMenu.h
 *
 *  Created on: 30.11.2018
 *      Author: dem34203
 */

#include "MenuBase.h"
#include "IdManager.h"
#include <string>

#ifndef BOOTSCREENMENU_H_
#define BOOTSCREENMENU_H_

namespace Interface
{

class BootscreenMenu : public MenuBase
{
public:
	typedef Util::IdManager::IdType 						IdType;
	typedef typename System::FourButtonArray::Button 		Button;
	typedef typename System::FourButtonArray::ButtonColor	ButtonColor;
	typedef typename System::Display::TextSize 				TextSize;
	typedef typename System::Display::LineDirection			Direction;

	BootscreenMenu(const std::string& name, const System::Manager& system);

	~BootscreenMenu() {};

	void init(void) const override;
	void enterMenu(void) const override;
	void exitMenu(void) const override;
	void showOverview(void) const override {};

private:
	const System::Timer& timer_;
	mutable IdType timerID_;
	mutable uint8_t iteration_;
	const System::FourButtonArray& channelButtons_;

	void progressBar(void) const;
}; //Class BootscreenMenu


BootscreenMenu::
BootscreenMenu(const std::string& name, const System::Manager& system) :
	MenuBase(name, system),
	timer_(system.timer()),
	timerID_(0),
	iteration_(0),
	channelButtons_(system.channelButtons())
{

}


void BootscreenMenu::
init(void) const
{

}


void BootscreenMenu::
enterMenu(void) const
{
	std::string title = "WAVEFORMGENERATOR 2.0";
	std::string nameTobi = "Frauenschlager Tobias";
	std::string nameTom = "Taugenbeck Thomas";
	std::string nameManu = "Dentgen Manuel";
	std::string subject = "VMCB/VMS Wintersemester 2018/2019";

	nameTobi[10] = 0xE4; // change a to ä

	display_.printText(30, headerHeight - 16, title, TextSize::medium);

	display_.printText(120 - ( ( nameTobi.length() * 5) / 2), 30, nameTobi, TextSize::small);
	display_.printText(120 - ( ( nameTom.length() * 5) / 2), 50, nameTom, TextSize::small);
	display_.printText(120 - ( ( nameManu.length() * 5) / 2), 70, nameManu, TextSize::small);
	display_.printText(120 - ( ( subject.length() * 5) / 2), 90, subject, TextSize::small);

	//outer Border
	display_.drawLine(0, 0, (xSize-1), Direction::horizontal);
	display_.drawLine(0, ((ySize*2)-1), (xSize-1), Direction::horizontal);
	display_.drawLine(0, 0, ((ySize*2)-1), Direction::vertical);
	display_.drawLine((xSize-1), 0, ((ySize*2)-1), Direction::vertical);

	//Headline
	display_.drawLine(0, headerHeight, (xSize-1), Direction::horizontal);

	//progress bar border
	display_.drawLine(20, 115, 200, Direction::horizontal);
	display_.drawLine(20, 121, 200, Direction::horizontal);
	display_.drawLine(20, 115, 4, Direction::vertical);
	display_.drawLine(220, 115, 4, Direction::vertical);

	display_.reloadContent();

	timerID_ = timer_.asyncRepeat(12ms, [this](){
		this->progressBar();
	});
}


void BootscreenMenu::
exitMenu(void) const
{
	display_.clearContent();
}


void BootscreenMenu::
progressBar(void) const
{
	display_.drawLine(21+iteration_, 116, 4, Direction::vertical);
	iteration_ ++;

	display_. reloadRows(114, 122);

	switch(iteration_)
	{
	case 8:  	displayButtons_.setButtonColor(Button::_1, ButtonColor::Green);   break;
	case 16: 	displayButtons_.setButtonColor(Button::_2, ButtonColor::Green);   break;
	case 24: 	displayButtons_.setButtonColor(Button::_3, ButtonColor::Green);   break;
	case 32: 	displayButtons_.setButtonColor(Button::_4, ButtonColor::Green);   break;
	case 40: 	channelButtons_.setButtonColor(Button::_3, ButtonColor::Green);   break;
	case 48: 	channelButtons_.setButtonColor(Button::_4, ButtonColor::Green);   break;
	case 56: 	channelButtons_.setButtonColor(Button::_2, ButtonColor::Green);   break;
	case 64: 	channelButtons_.setButtonColor(Button::_1, ButtonColor::Green);   break;
	case 72: 	channelButtons_.setButtonColor(Button::_1, ButtonColor::Orange);  break;
	case 80: 	channelButtons_.setButtonColor(Button::_2, ButtonColor::Orange);  break;
	case 88: 	channelButtons_.setButtonColor(Button::_4, ButtonColor::Orange);  break;
	case 96: 	channelButtons_.setButtonColor(Button::_3, ButtonColor::Orange);  break;
	case 104:	displayButtons_.setButtonColor(Button::_4, ButtonColor::Orange);  break;
	case 112: 	displayButtons_.setButtonColor(Button::_3, ButtonColor::Orange);  break;
	case 120: 	displayButtons_.setButtonColor(Button::_2, ButtonColor::Orange);  break;
	case 128: 	displayButtons_.setButtonColor(Button::_1, ButtonColor::Orange);  break;
	case 136: 	displayButtons_.setButtonColor(Button::_1, ButtonColor::Red);     break;
	case 144: 	displayButtons_.setButtonColor(Button::_2, ButtonColor::Red);     break;
	case 152: 	displayButtons_.setButtonColor(Button::_3, ButtonColor::Red);     break;
	case 160: 	displayButtons_.setButtonColor(Button::_4, ButtonColor::Red);     break;
	case 168: 	channelButtons_.setButtonColor(Button::_3, ButtonColor::Red);     break;
	case 176: 	channelButtons_.setButtonColor(Button::_4, ButtonColor::Red);     break;
	case 184: 	channelButtons_.setButtonColor(Button::_2, ButtonColor::Red);     break;
	case 192: 	channelButtons_.setButtonColor(Button::_1, ButtonColor::Red);     break;
	case 200:   timer_.abort(timerID_);
				displayButtons_.setButtonColor(Button::_1, ButtonColor::None, false);
				displayButtons_.setButtonColor(Button::_2, ButtonColor::None, false);
				displayButtons_.setButtonColor(Button::_3, ButtonColor::None, false);
				displayButtons_.setButtonColor(Button::_4, ButtonColor::None);
				channelButtons_.setButtonColor(Button::_3, ButtonColor::None, false);
				channelButtons_.setButtonColor(Button::_4, ButtonColor::None, false);
				channelButtons_.setButtonColor(Button::_2, ButtonColor::None, false);
				channelButtons_.setButtonColor(Button::_1, ButtonColor::None); break;
	default: break;
	}
}


} //Namespace Interface

#endif /* BOOTSCREENMENU_H_ */
