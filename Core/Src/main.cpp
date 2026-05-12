/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <main.h>
#include <screenHandle.hpp>
#include "screen.h"
#include <stdexcept>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/// RPM shift register stuff
#define RPM_REG 	GPIOA
#define RPM_LE 		GPIO_PIN_4
#define RPM_SCK 	GPIO_PIN_5
#define RPM_OE 		GPIO_PIN_8
#define RPM_MOSI 	GPIO_PIN_7

#define GEAR_REG 	GPIOB
#define GEAR_REG_OE GPIOA
#define GEAR_OE 	GPIO_PIN_11
#define GEAR_SCK 	GPIO_PIN_13
#define GEAR_LE 	GPIO_PIN_14
#define GEAR_MOSI 	GPIO_PIN_15



#define WARN_REG GPIOB
#define WARN_1		GPIO_PIN_10
#define WARN_2		GPIO_PIN_11
#define REV_LIM		GPIO_PIN_12

#define SCREEN_REG	GPIOB
#define SCREEN_SCL GPIO_PIN_6
#define SCREEN_SDA GPIO_PIN7

#define CAN_REGISTER GPIOB
#define CAN_RX GPIO_PIN_8
#define CAN_TX GPIO_PIN9

#define LED_MASK 0xFFFF

#define RPM_MAX 14000
#define ONE_LED RPM_MAX / 30

#define SCROLL_REG	GPIOA
#define SCROLL_PINA	GPIO_PIN_3
#define SCROLL_PINB GPIO_PIN_2

#define BTN GPIO_PIN_1

#define GEAR_ID 0x60E
#define LIMP_ID 0x61E
#define EOP_ID 0x608
#define VBAT_ID 0x60C
#define RPM_ID 0x600
#define OILTEMP_ID 0x604
#define ECT_ID 0x605
#define SPEED_ID 0x779
#define LAMBDA_ID 0x602
#define LAPTIME_ID 0x777
#define BRAKEBIAS_ID 0x779
#define DIFFBEST_ID 0x778
#define ROLLTIME_ID 0x780
#define SESSTIME_ID 0x780
#define BESTTIME_ID 0x778
#define BRAKES_ID 0x580
#define G_ID 0x580
#define TPS_ID 0x61B // byte 0 and 1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint16_t startnum[8] = {0b1110111, 0b0100010, 0b1101101, 0b1011101, 0b0011110, 0b1011011, 0b1111011, 0b0011111};
//uint16_t gears[7] = {0b0111000, 0b0100010, 0b1101101, 0b1011101, 0b0011110, 0b1011011,0b1111011};
uint8_t state = 0x00;
/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef TxReader; // i dont think I need this -> I in fact don't need this
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8]; //tag
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
extern void set_hi2c(I2C_HandleTypeDef hi2c);
void rpmLEDs(uint32_t ledseq);
void startupSeq();
void gearLEDs(uint8_t);
void rpmLEDs(uint32_t);

Rx_TypeDef gear(0x60E, INT, "");
Rx_TypeDef limpMode(0x61E, INT, "Limp: %d");
Rx_TypeDef EOP(0x608, FLOAT, "Oil Pressure: %.3f");
Rx_TypeDef vBat(0x60C, FLOAT, "vBat: %.3f");
Rx_TypeDef RPM(0x600, INT, "RPM: %d");
Rx_TypeDef oilTemp(0x604, FLOAT);
Rx_TypeDef ECT(0x605, FLOAT, "ECT: %.3f");
Rx_TypeDef lambda(0x602, FLOAT, "Lambda: %.3f");
Rx_TypeDef speed(0x779, INT,"Speed: %d MPH    ");
Rx_TypeDef laptime(0x777, TIME, "Time: %s");
Rx_TypeDef lapnum(0x777, INT, "Lap#: %d");
Rx_TypeDef brakeBias(0x779, FLOAT);
Rx_TypeDef diffBest(0x778, TIME);
Rx_TypeDef rollTime(0x780, TIME);
Rx_TypeDef sessTime(0x780, TIME);
Rx_TypeDef bestTime(0x778, TIME);
Rx_TypeDef fBrakeP(0x580, FLOAT, "Front P: %.3f");
Rx_TypeDef rBrakeP(0x580, FLOAT, "Rear P: %.3f");
Rx_TypeDef latG(0x581, FLOAT, "Lat G: %.3f");
Rx_TypeDef forwardG(0x581, FLOAT, "Forward G: %0.3f");
Rx_TypeDef tps(TPS_ID, INT, "Throttle: %d \%");




/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int RX_FLAG = 0;
volatile uint8_t rot_state = 0;
GEARS gearz[7] = {NEUTRAL, ONE, TWO, THREE, FOUR, FIVE, SIX};
ScreenHandler dashScreen;
volatile uint8_t next = 0;
volatile uint8_t back = 0;
volatile uint8_t btn = 0;
uint8_t setBright = 0;
float brightness = 0.05;
volatile uint32_t eop_rawval;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin)
{
	if(GPIO_pin == SCROLL_PINA) // to make things easy, lets start with just looking at when A is 0
	{
		rot_state = (rot_state << 2) | (HAL_GPIO_ReadPin(SCROLL_REG, SCROLL_PINA) << 1 | HAL_GPIO_ReadPin(SCROLL_REG, SCROLL_PINB));
		// lets look for 2 prev states redundancy
		uint8_t check = 0b00001111 & rot_state;
		if(check == 0b00000101) // cw turn
		{
			//HAL_GPIO_WritePin(WARN_REG, WARN_1, GPIO_PIN_SET);
			back = 1;
		}
		else if(check == 0b00000000) // ccw turn
		{
//			HAL_GPIO_WritePin(WARN_REG, WARN_2, GPIO_PIN_SET);
			next = 1;
		}
	}
	if(GPIO_pin == BTN)
	{
		btn = 1;
		HAL_GPIO_WritePin(WARN_REG, WARN_2, GPIO_PIN_SET);
	}

}
volatile int NUM_LEDS = 0;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	RX_FLAG = 1;
	uint32_t LEDMASK = 0xFFFFFFFc;
	uint32_t temp = 0b0;
	uint32_t temp2 = 0b0;
	float tempf = 0.0;
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK) Error_Handler();

	switch(RxHeader.StdId)
	{
	case GEAR_ID:
		gear._Update(RxData[3]);
		break;
	case LIMP_ID:
		limpMode._Update(RxData[0] << 8 | RxData[1]);
		break;
	case EOP_ID:
		EOP._UpdateFloat((float)(RxData[0] << 8 | RxData[1]) * 0.0145037738);
		eop_rawval = (float)(RxData[0] << 8 | RxData[1]);
		break;
	case VBAT_ID:
		vBat._UpdateFloat((float)(RxData[0] << 8 | RxData[1])/1000);
		break;
	case RPM_ID:
		RPM._Update(RxData[0] << 8 | RxData[1]);
		NUM_LEDS = (RPM.getValue() * 30) / RPM_MAX;
		rpmLEDs(~(LEDMASK>>NUM_LEDS));
		//rpmLEDs(~(LED_MASK >> NUM_LEDS));
		if(gear.getValue() >= 2 && gear.getValue() < 7)
		{
			gearLEDs(gearz[gear.getValue() - 2]);
		}
		break;
	case OILTEMP_ID:
		oilTemp._Update(RxData[2] << 8 | RxData[3]);
		break;
	case ECT_ID:
		ECT._UpdateFloat((float)((RxData[2] << 8 | RxData[3]) / 10) * 1.8 + 32);
		break;
	case LAMBDA_ID:
		lambda._UpdateFloat((float)(RxData[0] << 8 | RxData[1])/1000);
		break;
	case LAPTIME_ID:
		temp = 0b0;
		temp = RxData[0] | RxData[1] << 8 | RxData[2] << 16 | RxData[3] << 24;
		laptime._Update(temp);
		lapnum._Update(RxData[4]);
		break;
	case BRAKEBIAS_ID:
		tempf = (float)(RxData[4] << 8 | RxData[5]);
		brakeBias._Update(tempf / 10);
		speed._Update(RxData[6] << 8 | RxData[7]);
		break;
	case DIFFBEST_ID:
		temp = 0b0;
		temp = RxData[4] | RxData[5] << 8 | RxData[6] << 16 | RxData[7] << 24;
		diffBest._Update(temp);
		temp2 = 0b0;
		temp2 = RxData[0] | RxData[1] << 8 | RxData[2] << 16 | RxData[3] << 24;
		bestTime._Update(temp2);
		break;
	case ROLLTIME_ID:
		temp = 0b0;
		temp = RxData[0] | RxData[1] << 8 | RxData[2] << 16 | RxData[3] << 24;
		rollTime._Update(temp);
		temp2 = 0b0;
		temp = RxData[4] | RxData[5] << 8 | RxData[6] << 16 | RxData[7] << 24;
		diffBest._Update(temp2);
		break;
	case G_ID:
		tempf = (float)(RxData[0] | RxData[1] << 8);
		fBrakeP._UpdateFloat(tempf / 10);
		tempf = (float)(RxData[2] | RxData[3] << 8);
		rBrakeP._UpdateFloat(tempf/10);
		tempf = (float)(RxData[4] | RxData[5] << 8);
		latG._UpdateFloat(tempf/100);
		tempf = (float)(RxData[6] | RxData[7] << 8);
		forwardG._UpdateFloat(tempf / 100);
	case TPS_ID:
		tps._Update(RxData[0] | RxData[1] << 8);
		break;
	}
}

/**
  * @brief  Plays the startup sequence for when car turns on
  * @retval void
  */
void startupSeq()
{
	GEARS gears[] = {ONE, TWO, THREE, FOUR, FIVE, SIX};
	uint32_t snorp = 0xFFFFFFFc;
	for(int i = 1; i < 30; i++)
	{

		gearLEDs(gears[i/10]);
		rpmLEDs(~(snorp>>i));
		HAL_Delay(50);
	}
	for(int i = 1; i <= 30; i++){
		gearLEDs(gears[3 + i/10]);
		rpmLEDs(snorp << i);
		HAL_Delay(50);
	}

	return;
}

/**
  * @brief  Enables shift register LEDs
  * @retval void
  */
void LED_on()
{

}

/**
 * @brief turns all shift reg LEDs off
 */
void LED_off()
{

}

/**
 * @brief Sets RPM LEDs to uint32_t pattern
 * @param uint32_t ledseq
 * @retval none
 */
void rpmLEDs(uint32_t ledseq)
{
	//HAL_SPI_Transmit(&hspi1, (uint8_t*) &data, 4, HAL_MAX_DELAY);
	uint8_t n_led_drivers = 2;
	uint16_t data[n_led_drivers];

	// some fuckass bit twidling bruh
	uint16_t secondHalf1 = reverseBits((uint8_t)((0xFF00 &ledseq) >> 8));
	uint16_t secondHalf = (secondHalf1 << 8) | reverseBits((uint8_t)(0x00FF & ledseq));
	uint16_t firstHalf = (uint16_t)(ledseq >> 16);
	firstHalf = (reverseBits((uint8_t)((0xFF00 & firstHalf) >> 8))  << 8) | reverseBits((uint8_t)(0x00FF & firstHalf));
	data[0] = secondHalf;
	data[1] = firstHalf;
	HAL_GPIO_WritePin(RPM_REG, RPM_LE, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, (uint8_t*) &data, n_led_drivers * 2, HAL_MAX_DELAY);

	// pull LE high
	HAL_GPIO_WritePin(RPM_REG, RPM_LE, GPIO_PIN_SET);
	// pull LE low
	HAL_GPIO_WritePin(RPM_REG, RPM_LE, GPIO_PIN_RESET);
}

uint8_t reverseBits(uint8_t in_bits)
{
	uint8_t out_bits = 0x00;
	for(int i = 0; i < 8; i++)
	{
		out_bits <<= 1;
		if((in_bits & 0x01) == 1)
			out_bits |= 0x01;
		in_bits >>= 1;
	}
	return out_bits;
}

/**
 * @brief Sets gear LEDs to uint16_t pattern
 * @param uint16_t ledseq
 * @retval none
 */
void gearLEDs(uint8_t ledseq)
{
	uint8_t n_led_drivers = 1;
	//uint16_t data[n_led_drivers];
	//data[0] = ledseq;
	HAL_GPIO_WritePin(GEAR_REG, GEAR_LE, GPIO_PIN_RESET);

	// num data to be sent is n_led_drivers * 2 bc we are converting uint16_t array to a uint8_t array
	HAL_SPI_Transmit(&hspi2, &ledseq , 1, HAL_MAX_DELAY);

	// pull LE high
	HAL_GPIO_WritePin(GEAR_REG, GEAR_LE, GPIO_PIN_SET);
	// pull LE low
	HAL_GPIO_WritePin(GEAR_REG, GEAR_LE, GPIO_PIN_RESET);
	// pull OE pin low
	//HAL_GPIO_WritePin(GEAR_REG_OE, GEAR_OE, GPIO_PIN_RESET);
}


/**
 * @brief Set brightness amount withing range 1-10
 */
void setBrightness(float amount)
{
	uint32_t amt = (1-amount) * 65535;
	TIM1->CCR1 = amt;
	TIM1->CCR4 = amt;
	//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, amount/10);
}

int prev_btn = 1;
int cur_btn = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  // screen stuff



  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  set_hi2c(hi2c1);
  HAL_Delay(200);

  //HAL_GPIO_WritePin(LED_DRIVER_REG, OE_PIN, GPIO_PIN_SET);


  clear_display();
  display_FSAE_bootscreen();
  startupSeq();

  HAL_Delay(500);
  clear_display();
  set_funBitmap();

  CAN_FilterTypeDef canfilterconfig;
    /* LIMP MODE */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 1; // which filter bank to use from the assigned ones
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x60E << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0x60E << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

    /* EOP */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 2;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x608 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0x608 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* VBAT */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 3;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x60C << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0X60C << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* RPM AND SPEED */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 4;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x600 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0X600 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* OIL TEMP */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 5;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x604 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0X604 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* ECT */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 6;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x605 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0X605 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* LAMBDA */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 7;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x602 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0X602 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  /* GEAR */
  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 8;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x60E << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0X60E << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 9;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x777 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0x777 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 10;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x779 << 5;
  canfilterconfig.FilterIdLow = 0;
  canfilterconfig.FilterMaskIdHigh = 0x779 << 5;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 11;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = 0x778 << 5;
  canfilterconfig.FilterIdLow =  0x780 << 5;
  canfilterconfig.FilterMaskIdHigh = 0x778 << 5;
  canfilterconfig.FilterMaskIdLow =  0x780 << 5;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 12;
  canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfilterconfig.FilterIdHigh = G_ID << 5;
  canfilterconfig.FilterIdLow = TPS_ID << 5;
  canfilterconfig.FilterMaskIdHigh = G_ID << 5;
  canfilterconfig.FilterMaskIdLow = TPS_ID << 5;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_16BIT;
  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);


  //TIM1->CCR1 = 6553;
  //setBrightness(1);
  TIM1->BDTR = TIM_BDTR_MOE; // dont remove or ill kill myself
  if(HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) Error_Handler(); // enable CAN interrupts

  Rx_TypeDef* warmupstuff[] = {&ECT, &EOP, &vBat, &lambda};
  Rx_TypeDef* drivingstuff[] = {&speed, &ECT, &limpMode, &tps};
  Rx_TypeDef* lapstuff[] = {&laptime, &lapnum};
  Rx_TypeDef* brakestuff[] = {&fBrakeP, &rBrakeP};
  Rx_TypeDef* gstuff[] {&latG, &forwardG};
  ScreenPanel warmupScreen(" WARMUP ", warmupstuff);
  ScreenPanel drivingScreen(" DRIVING ", drivingstuff);
  ScreenPanel laptimeScreen(" ENDURANCE ", lapstuff);
  ScreenPanel brakeScreen(" BRAKES ", brakestuff);
  ScreenPanel gScreen(" G FORCE ", gstuff); // hehe like g spot

  dashScreen.addScreen(&warmupScreen);
  dashScreen.addScreen(&drivingScreen);
  dashScreen.addScreen(&laptimeScreen);
  dashScreen.addScreen(&brakeScreen);
  dashScreen.addScreen(&gScreen);

  dashScreen.startScreen();
  rpmLEDs(0);
  gearLEDs(0);



  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_CAN_Start(&hcan);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  setBrightness(0.05);
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  cur_btn = HAL_GPIO_ReadPin(SCROLL_REG, BTN);
	  if(cur_btn == 1 && prev_btn == 0)
		  btn = 1;
	  prev_btn = cur_btn;

	  if(setBright == 1)
	  {
		  rpmLEDs(0xFFFFFFFF);
		  gearLEDs(0xFF);
		  if(next)
		  {
			  brightness += 0.025;
			  setBrightness(brightness);
			  next = 0;
		  }
		  else if(back)
		  {
			  if(brightness > 0.05)
				  brightness -= 0.025;
			  setBrightness(brightness);
			  back = 0;
		  }
		  if(btn){
			  btn = 0;
			  setBright = 0;
			  clear_display();
			  __HAL_CAN_ENABLE_IT(&hcan, CAN_RX_FIFO0);
		  }

	  }
	  else
	  {

		  if(RX_FLAG) {
			  dashScreen.handle();
		  	  if(ECT.getValue() >= 113) HAL_GPIO_WritePin(WARN_REG, WARN_1, GPIO_PIN_SET);
		  	  if(ECT.getValue() < 105) HAL_GPIO_WritePin(WARN_REG, WARN_1, GPIO_PIN_RESET);
		  	  if(((RPM.getValue() <= 4000 && EOP.getValue() < 6)
		  			 || ((RPM.getValue() * 0.0058) > EOP.getValue()))
		  			  && RPM.getValue() != 0) HAL_GPIO_WritePin(WARN_REG, WARN_2, GPIO_PIN_SET);
		  	  else HAL_GPIO_WritePin(WARN_REG, WARN_2, GPIO_PIN_RESET);
		  	  if(NUM_LEDS == 30){
		  		  HAL_GPIO_WritePin(WARN_REG, REV_LIM, GPIO_PIN_SET);
		  	  }

		  	  RX_FLAG = 0;

		  }
		  if(next == 1)
		  {
			  dashScreen.nextScreen();
			  dashScreen.handle();
			  next = 0;
		  }
		  else if(back == 1)
		  {
			  dashScreen.prevScreen();
			  dashScreen.handle();
			  back = 0;
		  }
		  if(btn == 1)
		  {
			  if (dashScreen.getIndex() == 0)
			  {
				  __HAL_CAN_DISABLE_IT(&hcan, CAN_RX_FIFO0);
				  setBright = 1;
				  clear_display();
				  move_cursor(0,1);
				  write_byte(129);
				  write_bytes(" SET BRIGHTNESS ");
				  write_byte(129);

				  move_cursor(0,2);
				  for(int i = 0; i < 20; i++){
					  write_byte(128);
				  }
				  //HAL_GPIO_WritePin(WARN_REG, WARN_2, GPIO_PIN_SET);

			  }
			  btn = 0;
		  }
	  }
  }
  /* USER CODE END 3 */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 16;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_ETRMODE2;
  sClockSourceConfig.ClockPolarity = TIM_CLOCKPOLARITY_NONINVERTED;
  sClockSourceConfig.ClockPrescaler = TIM_CLOCKPRESCALER_DIV1;
  sClockSourceConfig.ClockFilter = 0;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB12 PB14 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
//  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
//  HAL_NVIC_EnableIRQ(EXTI1_IRQn);


  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
