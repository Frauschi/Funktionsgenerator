#ifndef MENUCONTROLLER_H_
#define MENUCONTROLLER_H_

#include "MenuBase.h"
#include "MainMenu.h"
#include "ChannelMenu.h"
#include <cstdint>
#include <array>
#include <functional>

namespace Interface
{


enum MenuNames : std::uint8_t{
	_Mainmenu,
	_Channel1,
	_Channel2,
	_Bootscreen,
	Menu_Count
};


template <std::size_t TMenuCount>
class MenuController
{
public:
	typedef typename System::FourButtonArray::Button 					Button;
	typedef typename System::FourButtonArray::ButtonColor				ButtonColor;

	//Constructor
	explicit MenuController(const System::Manager& system);

	//Destructor
	~MenuController(void);

	void changeMenuTo(MenuNames nextMenu) const;

	void addMenu(const MenuBase* newMenuBase, MenuNames const newMenuName) const;

	void init(void) const;

	void channel1SelectButtonPressed(void) const;
	void channel2SelectButtonPressed(void) const;
	void channel1ActivateButtonPressed(void) const;
	void channel2ActivateButtonPressed(void) const;

private:

	mutable std::array<const MenuBase*, MenuNames::Menu_Count> menus_;
	mutable MenuNames currentMenu_;

	const System::FourButtonArray& displayButtons_;
	const System::FourButtonArray& channelButtons_;

};	//Class UserInterface


template <typename std::size_t TMenuCount>
MenuController<TMenuCount>::
MenuController(const System::Manager& system) :
	currentMenu_(_Channel2),
	displayButtons_(system.displayButtons()),
	channelButtons_(system.channelButtons())
{

}


template <typename std::size_t TMenuCount>
MenuController<TMenuCount>::
~MenuController(void)
{

}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
changeMenuTo(MenuNames const nextMenu) const
{
	if(currentMenu_ != nextMenu)
	{
		menus_[currentMenu_]->exitMenu();

		if(nextMenu == _Mainmenu)
		{
			channelButtons_.setButtonColor(Button::_1, ButtonColor::Green);
			channelButtons_.setButtonColor(Button::_2, ButtonColor::Red);
			displayButtons_.setButtonColor(Button::_1, ButtonColor::Orange, false);
			displayButtons_.setButtonColor(Button::_2, ButtonColor::Orange, false);
			displayButtons_.setButtonColor(Button::_3, ButtonColor::Orange, false);
			displayButtons_.setButtonColor(Button::_4, ButtonColor::Orange);
		}
		else if(nextMenu == _Channel1)
		{
			channelButtons_.setButtonColor(Button::_1, ButtonColor::Orange);
			channelButtons_.setButtonColor(Button::_2, ButtonColor::Red);
			displayButtons_.setButtonColor(Button::_1, ButtonColor::Green, false);
			displayButtons_.setButtonColor(Button::_2, ButtonColor::Green, false);
			displayButtons_.setButtonColor(Button::_3, ButtonColor::Green, false);
			displayButtons_.setButtonColor(Button::_4, ButtonColor::Green);
		}
		else if(nextMenu == _Channel2)
		{
			channelButtons_.setButtonColor(Button::_1, ButtonColor::Green);
			channelButtons_.setButtonColor(Button::_2, ButtonColor::Orange);
			displayButtons_.setButtonColor(Button::_1, ButtonColor::Red, false);
			displayButtons_.setButtonColor(Button::_2, ButtonColor::Red, false);
			displayButtons_.setButtonColor(Button::_3, ButtonColor::Red, false);
			displayButtons_.setButtonColor(Button::_4, ButtonColor::Red);
		}

		currentMenu_ = nextMenu;

		menus_[currentMenu_]->enterMenu();
	}
}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
addMenu(const MenuBase* newMenuBase, MenuNames const newMenuName) const
{
	menus_[newMenuName] = newMenuBase;
}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
init(void) const
{
	channelButtons_.addHandlerForButton(Button::_1, [&]() {
		channel1SelectButtonPressed();
	});
	channelButtons_.addHandlerForButton(Button::_2, [&]() {
		channel2SelectButtonPressed();
	});
	channelButtons_.addHandlerForButton(Button::_3, [&]() {
		channel1ActivateButtonPressed();
	});
	channelButtons_.addHandlerForButton(Button::_4, [&]() {
		channel2ActivateButtonPressed();
	});
}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
channel1SelectButtonPressed(void) const
{
	if(currentMenu_ == _Mainmenu || currentMenu_ == _Channel2)
	{
		changeMenuTo(_Channel1);
	}
	else if(currentMenu_ == _Channel1)
	{
		changeMenuTo(_Mainmenu);
	}
}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
channel2SelectButtonPressed(void) const
{
	if(currentMenu_ == _Mainmenu || currentMenu_ == _Channel1)
	{
		changeMenuTo(_Channel2);
	}
	else if(currentMenu_ == _Channel2)
	{
		changeMenuTo(_Mainmenu);
	}
}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
channel1ActivateButtonPressed(void) const
{
	if(menus_[_Channel1]->isActivated() == true)
	{
		menus_[_Channel1]->changeActivationTo(false);
		channelButtons_.setButtonColor(Button::_3, ButtonColor::None);
	}
	else
	{
		menus_[_Channel1]->changeActivationTo(true);
		channelButtons_.setButtonColor(Button::_3, ButtonColor::Green);
	}

	if(currentMenu_ == _Channel1)
	{
		menus_[_Channel1]->reloadHeader();
	}
}


template <typename std::size_t TMenuCount>
void MenuController<TMenuCount>::
channel2ActivateButtonPressed(void) const
{
	if(menus_[_Channel2]->isActivated() == true)
	{
		menus_[_Channel2]->changeActivationTo(false);
		channelButtons_.setButtonColor(Button::_4, ButtonColor::None);
	}
	else
	{
		menus_[_Channel2]->changeActivationTo(true);
		channelButtons_.setButtonColor(Button::_4, ButtonColor::Red);
	}

	if(currentMenu_ == _Channel2)
	{
		menus_[_Channel2]->reloadHeader();
	}
}


} //namespace Interface

#endif //MENUCONTROLLER_H_
