/*
 *   Created on: Apr 18, 2025
 *      Author: Phoenix Cardwell
 *      UCONN Formula SAE 2025
 */

#ifdef __cplusplus
#include <screenHandle.hpp>
#include <stdlib.h>
#include "screen.h"
#include <cstring>

/*!
 * @brief Displays the title of the screen
 */
void ScreenPanel::titleScreen(){
	clear_display();
	move_cursor(5,1);
	write_byte(129);
	write_bytes_const(name);
	write_byte(129);

	move_cursor(0,2);
	for(int i = 0; i < 20; i++){
		write_byte(128);
	}

	HAL_Delay(500);
}

/*!
 * @brief displays the data for that screen
 */
void ScreenPanel::Display()
{
	//clear_display();

	for(int i = 0; i < 4; i++)
	{
		//int col = 10 * (i/4);
		//int line = i%4;
		move_cursor(0,i);
		char buf[20];
		switch(data[i]->getType())
		{
		case FLOAT:
			sprintf(buf, data[i]->getLabel(), data[i]->getFloat());
			break;
		case INT:
			sprintf(buf, data[i]->getLabel(), data[i]->getValue());
			break;
		case TIME:
			char* time = formatTime(data[i]->getValue());
			snprintf(buf,sizeof(buf), data[i]->getLabel(), time);
		}
		write_bytes(buf);
	}

}

void ScreenHandler::startScreen(){
	_screens[_curScreen]->titleScreen();
	clear_display();
	_screens[_curScreen]->Display();
}
void ScreenHandler::nextScreen(){
	_curScreen++; // go to next screen
	_curScreen %= (int)_screens.size(); // if past last screen go back to first screen
	_screens[_curScreen]->titleScreen();
	clear_display();

	_screens[_curScreen]->Display(); // update display ?
	// add edge case to loop back to beginning
}

void ScreenHandler::prevScreen(){
	_curScreen--; // go to last screen
	if(_curScreen < 0) _curScreen = (int) _screens.size() - 1; // if past first screen go to last screen
	_screens[_curScreen]->titleScreen();
	clear_display();
	_screens[_curScreen]->Display(); // update display ?
	// add edge case to loop back to beginning
}

void ScreenHandler::handle()
{
	_screens[_curScreen]->Display();
}

void ScreenHandler::addScreen(ScreenPanel* screen)
{
	_screens.push_back(screen);
}


char* formatTime(int ms) {
    bool isNegative = ms < 0;
    ms = std::abs(ms);

    int hours = ms / 3600000;
    ms %= 3600000;
    int minutes = ms / 60000;
    ms %= 60000;
    int seconds = ms / 1000;
    ms %= 1000;

    char buffer[24]; // Extra space for minus sign and null terminator
    snprintf(
        buffer, sizeof(buffer), "%s%d:%02d:%02d.%03d",
        isNegative ? "-" : "",
        hours, minutes, seconds, ms
    );

    return buffer;
}

#endif
