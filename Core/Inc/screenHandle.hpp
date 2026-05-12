/*! @file
 *
 *  Created on: Apr 18, 2025
 *      Author: Phoenix Cardwell
 *      UCONN Formula SAE 2025
 */
#ifndef __SCREENHANDLE_HPP
#define __SCREENHANDLE_HPP

#include <main.h>
/** \cond */
#ifdef __cplusplus
/** \endcond */
#include <vector>
#include <string>
#include <RxClass.h>
#include <stdio.h>
#include <main.h>

class ScreenHandler;
class ScreenPanel;
class Rx_TypeDef;
class Rx_LapStats;
char* formatTime(int ms);

/*!
 * @class ScreenHandler
 * @brief A class to handle the screen.
 *
 * Class that holds ScreenPanel classes and handles switching
 * between screens and telling the ScreenPanel to update it's
 * values.
 */

class ScreenHandler
{
public:
	ScreenHandler() { _curScreen = 0; };
	//ScreenHandler(std::vector<ScreenPanel&> screens) : _screens(screens) {}
	~ScreenHandler() {}

	void startScreen();
	void handle();
	void update();
	void nextScreen();
	void prevScreen();
	void addScreen(ScreenPanel* screen);
	int getIndex() { return _curScreen; }
private:
	int _curScreen; // pointer to current screen

	// wait maybe i want curscreen to be an iterator
	std::vector<ScreenPanel*> _screens; // array of all screens
};


class ScreenPanel
{
public:
	ScreenPanel(const char* name, Rx_TypeDef** Data) : name(name), data(Data) {}
	~ScreenPanel() {}
	// what do I need here?
	void Display(); // displays to LCD screen
	void titleScreen();
	void ButtonAction(); // what does the button do when you click it
private:
	Rx_TypeDef** data;
	const char* name;
};

/** \cond */
extern "C" {


}
#endif
/** \endcond */
#endif
