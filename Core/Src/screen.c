/*
*  Created on: Apr 18, 2025
 *      Author: Ben Deusch
 *      UCONN Formula SAE 2025
*/

#include "screen.h"


I2C_HandleTypeDef screen_hi2c1;

const uint8_t fun_bitmap[2][8] = {
		{
				0b00011001,
				0b00011001,
				0b01100110,
				0b01100110,
				0b00011001,
				0b00011001,
				0b01100110,
				0b01100110
		},
//		{
//				0b01000000,
//				0b01110011,
//				0b01111111,
//				0b01111111,
//				0b01111111,
//				0b01001100,
//				0b01000000,
//				0b01000000
//		},
		{

				0b00100000,
				0b00111001,
				0b00111111,
				0b00111111,
				0b00111111,
				0b00100110,
				0b00100000,
				0b00100000

		}

};

const uint8_t uconn_fsae_bitmap[8][8] = {
  {
    0b00111111,
    0b00100000,
    0b00100000,
    0b00010000,
    0b00001100,
    0b00000011,
    0b00000110,
    0b00011000,
  },
  {
    0b00111100,
    0b00000010,
    0b00000010,
    0b00000111,
    0b00011100,
    0b00110000,
    0b00000000,
    0b00000000,
  },
  {
    0b00000001,
    0b00001110,
    0b00111000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
  },
  {
    0b00100000,
    0b00100111,
    0b00101100,
    0b00101111,
    0b00100000,
    0b00100111,
    0b00011000,
    0b00000000,
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00100001,
    0b00111110,
    0b00000000,
    0b00000000,
    0b00000000,
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00111000,
    0b00001111,
    0b00000000,
    0b00000000,
    0b00000000,
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00111100,
    0b00001111,
    0b00000000,
    0b00000000,
  },
  {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00110000,
    0b00001110,
    0b00000011,
  },
};


void set_hi2c(I2C_HandleTypeDef hi2c){
	screen_hi2c1 = hi2c;
}

void write_byte(uint8_t byte) {
//  Wire.beginTransmission(0x2A);
//  Wire.write(byte);
//  Wire.endTransmission();
//  delay(1);
	uint16_t DevAddress = 0x2A << 1; // Shifted left for HAL function (8-bit address format)
	uint8_t data = byte;

	// Send the byte
	if (HAL_I2C_Master_Transmit(&screen_hi2c1, DevAddress, &data, 1, 100) == HAL_OK)
	{
	  // Data successfully sent
	  //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0); // Example feedback action (LED toggle)
	}
	else
	{
	  // Error occurred during transmission
	  //Error_Handler();
	}
	HAL_Delay(5);
}

void write_bytes(char* input) {
  for (int i = 0; input[i] != '\0'; i++) {
    write_byte(input[i]);
  }

}

void write_bytes_const(const char* input) {
  for (int i = 0; input[i] != '\0'; i++) {
    write_byte(input[i]);
  }

}

void clear_display() {
  write_byte(0x0C);
  HAL_Delay(5);
}

void hide_cursor() {
  write_byte(0x04);
}

void move_cursor(uint8_t column, uint8_t row) {
	// upper left position is (0,0)
	// lower-right position is (19,3)
  write_byte(0x11);
  write_byte(column);
  write_byte(row);
}

void set_FSAE_bitmap() { // original signature: void set_bitmap(uint8_t bitmap[8][8])
  for (uint8_t character = 0; character < 8; character++) {
    // set bitmap instruction (0x19), then the character (0-7)
    write_byte(0x19);
    write_byte(character);
    for (uint8_t line = 0; line < 8; line++) {
      uint8_t bytes = uconn_fsae_bitmap[character][line];
      write_byte(bytes);
    }
  }
}

void set_funBitmap() {
	write_byte(0x19);
	write_byte(0);
	for(uint8_t line = 0; line < 8; line++) {
		uint8_t bytes = fun_bitmap[0][line];
		write_byte(bytes);
	}
	write_byte(0x19);
	write_byte(1);
	for(uint8_t line = 0; line < 8; line++) {
		uint8_t bytes = fun_bitmap[1][line];
		write_byte(bytes);
	}
}

void write_bitmap() {
  write_byte(128);
  write_byte(129);
  write_byte(130);
  write_byte(131);
  write_byte(132);
  write_byte(133);
  write_byte(134);
  write_byte(135);
}


void display_FSAE_bootscreen() {
	set_FSAE_bitmap();
	move_cursor(6,1);
	write_bitmap();
	hide_cursor();
	move_cursor(5,2);
	write_bytes("UConn FSAE");
	HAL_Delay(100);
}


void reboot()
{
	write_byte(0x1A);
	write_byte(016);
}
