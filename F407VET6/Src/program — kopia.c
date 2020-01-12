#include "main.h"
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "komendy.h"
#include "i2c_scan_hal.h"
#include "program.h"
#include "SD_reg.h"



extern I2C_HandleTypeDef hi2c1;
//extern I2C_HandleTypeDef hi2c2;

//extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart1;

extern RTC_HandleTypeDef hrtc;
extern SD_HandleTypeDef hsd;


static uint8_t Flaga5ms = 0;
static uint8_t Flaga250ms = 0;
static uint8_t Flaga500ms = 0;
static uint8_t Flaga1000ms = 0;

float pressure = 1024.25;
float temperature = 25.75;


// RTC set--------------------------------------------------------------------------------------------
void Set_SD_TimeDate () {
	RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
	uint8_t result;
	char char_buff[BUFF_MAX_SIZE];
	char buffer[10];
	
	result = SD_read_conf (char_buff);
	if (result == 1) 	HAL_GPIO_WritePin(GPIOA, D3_Pin, GPIO_PIN_RESET);
	
	Take_Num_String(buffer, char_buff, BUFF_MAX_SIZE);
	
	if(buffer[0] == 0x00) return; //If SET = 0 or no config file return
	
  sTime.Hours = buffer[1];
  sTime.Minutes = buffer[2];
  sTime.Seconds = buffer[3];
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
 
	sDate.Date = buffer[4];
  sDate.Month = buffer[5];
  sDate.Year = buffer[6];
	sDate.WeekDay = buffer[7];

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
	
	Change_SD_Set(); // Changing set in config file after setting Time Date
	
}

//Alarm IT set --------------------------------------------------------------------------------------------
void Set_RTC_Alarm () {
	
	RTC_AlarmTypeDef sAlarm;

	sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

//Helping functions --------------------------------------------------------------------------------------------
// Delay --------------------------------------------------------------------------------------------
void Delay(uint16_t ms) {
	HAL_Delay(ms);
}

// Taking number bytes from string
uint8_t Take_Num_String(char * num_buff, char * char_buff, size_t size) { // max 255 bytes
	uint8_t bit_nmb = 2;
	uint8_t count = 0;
	int i;
	char num;

	for (i = 0; i < size; i++) {
		if(char_buff[i] > 47 && char_buff[i] < 58) {
			bit_nmb--;
			num += (char_buff[i] - 48)<<(4*bit_nmb);
		}
		
		if(bit_nmb == 0) {
			num_buff[count] = num;
			count ++;
			bit_nmb = 2;
			num = 0;
		}
	}
	
	return count;
}


// Meas save
void Meas_Save (float press, float temp) {
	static char buffer [40];
	uint16_t size = 0;
	uint8_t result = 0;
	static RTC_DateTypeDef Date;
	static RTC_TimeTypeDef Time;
	
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BCD);
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BCD);
	
	size = sprintf(buffer, "%x %x %x %x %x  %.2f %.2f\r\n", Date.Date, Date.Month, Date.Year, Time.Hours, Time.Minutes, press, temp);
	result = SD_add_reg(buffer, size);
	if(result == 1) HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_RESET);
	
}



// Init function (before while)
void setup (void) {

	
	HAL_GPIO_WritePin(GPIOA, D3_Pin, GPIO_PIN_SET);  //Turn off LEDs
	HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_SET);

	
	Set_SD_TimeDate ();

//	SD_demo ();
}


//main loop
void loop (void) {
	
	
	
	if (Flaga250ms == 0x01) {
		Flaga250ms = 0;
								
	}
		
	if (Flaga1000ms == 0x01) {
		Flaga1000ms = 0;
								
//		Meas_Save(pressure, temperature);
		
		__NOP();
	}
	
//	__WFI();
//	HAL_Delay (100);
}


void SysTick_Flags () {
	static uint16_t licznik5 = 0;
	static uint16_t licznik250 = 0;
	static uint16_t licznik500 = 0;
	static uint16_t licznik1000 = 0;
	
	if(licznik5 == 5) {
		Flaga5ms = 1;
		licznik5 = 0;
	}
	
	if(licznik250 == 250) {
		Flaga250ms = 1;
		licznik250 = 0;
	}

	if(licznik500 == 500) {
		Flaga500ms = 1;
		licznik500 = 0;
	}

	if(licznik1000 == 1000) {
		Flaga1000ms = 1;
		licznik1000 = 0;
	}
	
	licznik5++;
	licznik250++;
	licznik500++;
	licznik1000++;
}



