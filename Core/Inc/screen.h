#ifndef __SCREEN_H
#define __SCREEN_H
#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"


void set_hi2c(I2C_HandleTypeDef hi2c);
void write_byte(uint8_t);  // Writes one byte to the display
void write_bytes(char* input);  // Takes a string, writes all characters to the display, one at a time
void write_bytes_const(const char* input);  // Takes a string, writes all characters to the display, one at a time
void clear_display();  // Clears the display
void hide_cursor();  // Hides the cursor
void move_cursor(uint8_t column, uint8_t row); // Moves the cursor to given indices
void set_FSAE_bitmap(); // Sets the bitmap to the FSAE one
void set_funBitmap();

void display_FSAE_bootscreen();
void write_bitmap();  // Writes current bitmap to the display
void reboot();
//void set_hi2c(I2C_HandleTypeDef hi2c);
#ifdef __cplusplus
}
#endif
#endif
