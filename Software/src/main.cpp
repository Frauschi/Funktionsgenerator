
#include <chrono>
#include <functional>

#include "SystemManager.h"
#include "ChannelMenu.h"
#include "MainMenu.h"
#include "BootscreenMenu.h"
#include "MenuController.h"


using namespace std::chrono_literals;

int main(void)
{
	/* Create a reference to the system */
	auto& system_ = System::instance();

	/* Create objects for UserInterface */
	Interface::MenuController<Interface::MenuNames::Menu_Count> menuController_(system_);
	Interface::MainMenu mainmenu_("Mainmenu", system_, 0x0001);
	Interface::ChannelMenu channel1_("Channel 1", SignalGeneration::Output::Ch1, system_, 0x0002);
	Interface::ChannelMenu channel2_("Channel 2", SignalGeneration::Output::Ch2, system_, 0x0003);
	Interface::BootscreenMenu bootscreen_("Waveformgenerator 2.0", system_);


	/* Reset display */
	system_.display().reset();

	/* Delayed initialisation of further components */
	system_.timer().asyncWait(150ms, [&](){

		/* Create a reference to the system */
		auto& system_ = System::instance();

		/* Start a heartbeat signal to pulse the status LEDs */
		system_.heartbeat().init(500ms);

		/* 	Create a new Pin map object for the port expander
		 * 	Will be copied into the button array, so we can delete this object at the
		 * 	end of the initialize() method
		 * 	This object will be used for both FourButtonArrays:
		 * 	first we set the pin values for the first portExpander, copy the map in
		 * 	the local storage in the class and then modify the pin values for the
		 * 	second portExpander
		 */
		System::FourButtonArray::PinMap portExpanderPinMap;

		portExpanderPinMap.Button1Press	= 9;
		portExpanderPinMap.Button1Red	= 11;
		portExpanderPinMap.Button1Green = 10;
		portExpanderPinMap.Button2Press	= 7;
		portExpanderPinMap.Button2Red	= 8;
		portExpanderPinMap.Button2Green = 6;
		portExpanderPinMap.Button3Press	= 5;
		portExpanderPinMap.Button3Red	= 4;
		portExpanderPinMap.Button3Green = 3;
		portExpanderPinMap.Button4Press	= 2;
		portExpanderPinMap.Button4Red	= 1;
		portExpanderPinMap.Button4Green = 0;
		system_.displayButtons().initialize(portExpanderPinMap);

		/* Modify pinMap for portExpander 2 */
		portExpanderPinMap.Button1Press	= 15;
		portExpanderPinMap.Button1Red	= 13;
		portExpanderPinMap.Button1Green = 14;
		portExpanderPinMap.Button2Press	= 10;
		portExpanderPinMap.Button2Red	= 8;
		portExpanderPinMap.Button2Green = 9;
		portExpanderPinMap.Button3Press	= 0;
		portExpanderPinMap.Button3Red	= 1;
		portExpanderPinMap.Button3Green = 2;
		portExpanderPinMap.Button4Press	= 7;
		portExpanderPinMap.Button4Red	= 5;
		portExpanderPinMap.Button4Green = 6;
		system_.channelButtons().initialize(portExpanderPinMap);

		/* Initialize the display */
		system_.display().init();

		menuController_.addMenu(&mainmenu_, Interface::MenuNames::_Mainmenu);
		menuController_.addMenu(&channel1_, Interface::MenuNames::_Channel1);
		menuController_.addMenu(&channel2_, Interface::MenuNames::_Channel2);
		menuController_.addMenu(&bootscreen_, Interface::MenuNames::_Bootscreen);
		menuController_.changeMenuTo(Interface::MenuNames::_Bootscreen);
	});

	system_.timer().asyncWait(250ms, [&](){
		system_.frequencyController().initialize();;
	});


	system_.timer().asyncWait(550ms, [&](){
		/* Initialize the menus and add them to the controller */
		mainmenu_.init();
	});

	system_.timer().asyncWait(850ms, [&](){
		channel1_.init();
	});

	system_.timer().asyncWait(1150ms, [&](){
		channel2_.init();
	});


	/* Change to main menu after 3 seconds in the boot screen */
	system_.timer().asyncWait(3s, [&](){
		menuController_.init();
		menuController_.changeMenuTo(Interface::MenuNames::_Mainmenu);

	});

	/* Start main program execution */
	system_.eventLoop().run();
}
