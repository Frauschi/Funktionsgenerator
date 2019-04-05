#ifndef MENUBASE_H_
#define MENUBASE_H_

#include "SystemManager.h"
#include <string>
#include <cstring>
#include <vector>

namespace Interface
{

class SubmenuBase;

class MenuBase
{
public:
	typedef typename System::FourButtonArray::Button 		Button;
	typedef typename System::Display::TextSize 				TextSize;

	//Constructor
	MenuBase(const std::string& name, const System::Manager& system, std::uint16_t const eepromStorageOrder = 0x0000);
	//Destructor
	virtual ~MenuBase();

	virtual void init(void) const = 0;
	virtual void enterMenu(void) const = 0;
	virtual void exitMenu(void) const = 0;
	virtual void showOverview(void) const = 0;

	virtual bool isActivated(void) const {return false;};
	virtual void changeActivationTo(bool activation) const { static_cast<void>(activation); };
	virtual void reloadHeader(void) const {};

	uint32_t getEepromBaseAdress(void) const;

protected:
	const std::string name_;

	const System::Display& display_;
	const System::Encoder& encoder_;
	const System::FourButtonArray& displayButtons_;
	const System::EEPROM& eeprom_;

	mutable SubmenuBase const* currentSubMenu_;

	const std::uint16_t eepromBaseAdress_;

}; //Class MenuBase

//---------------------------------------------------------------------------------------
//----------------------- Implementation of Class 'MenuBase' ----------------------------

MenuBase::
MenuBase(const std::string& name, const System::Manager& system, std::uint16_t const eepromStorageOrder) :
	name_(name),
	display_(system.display()),
	encoder_(system.encoder()),
	displayButtons_(system.displayButtons()),
	eeprom_(system.eeprom()),
	currentSubMenu_(nullptr),
	eepromBaseAdress_((eepromStorageOrder<<5))
{

}


MenuBase::
~MenuBase(void)
{

}


uint32_t MenuBase::
getEepromBaseAdress(void) const
{
	return eepromBaseAdress_;
}


} //Namespace Interface

#endif	//MENUBASE_H_
