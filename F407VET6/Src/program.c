/**
* Wojciech Dron
* Praca dyplomowa inzynierska
* Mikroprocesorowy Rejestrator Cisnienia Atmosferycznego 2019
*/

#include "main.h"
#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "komendy.h"
#include "i2c_scan_hal.h"
#include "program.h"
#include "SD_reg.h"
#include "lcd_hd44780_i2c.h"
#include "bmp280.h"


extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

extern RTC_HandleTypeDef hrtc;

extern SD_HandleTypeDef hsd;
extern DMA_HandleTypeDef hdma_sdio_tx;
extern DMA_HandleTypeDef hdma_sdio_rx;

extern TIM_HandleTypeDef htim1;

extern UART_HandleTypeDef huart1;

BMP280_HandleTypedef bmp280;


static uint8_t Flaga5ms = 0;
static uint8_t Flaga250ms = 0;
static uint8_t Flaga500ms = 0;
static uint8_t Flaga1000ms = 0;
uint8_t btn_repeat_save = 0; //zabezpieczenie przed filkukrotnym wcisnieciem
uint16_t time_ensure = 0;

float pressure = 1024.25;
float temperature = 25.75;
float humidity = 0;

uint8_t stopien[8] = {0x1C,	0x14,	0x1C,	0x00,	0x00,	0x00,	0x00,	0x00,};


uint8_t buffer_lcd_time[33] = "Hello"; //display mode 0
uint16_t size_lcd_time = 5;
uint8_t buffer_lcd_meas[33] = "Hello"; //display mode 1
uint16_t size_lcd_meas = 7;
uint8_t buffer_lcd_reg[33] = "No meas saved"; //display mode 2
uint16_t size_lcd_reg = 13;
uint8_t buffer_lcd_alm[33] = "Alarm not set"; //display mode 3
uint16_t size_lcd_alm = 13;
uint8_t buffer_lcd_error [33] = "Rejest cisn+temp\nWojciech Dron"; //display mode 4
uint16_t size_lcd_error = 30;

uint8_t display_mode = 1;

RTC_DateTypeDef Date;
RTC_TimeTypeDef Time;

// RTC set--------------------------------------------------------------------------------------------
// Wylaczyc ustawianie daty w mainie
void RTC_init_SD () {
	RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
	
	uint8_t result;
	char char_buff[BUFF_MAX_SIZE];
	char buffer[10];
	
   /**Initialize RTC Only  */
	hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
	
	

	// Setting time and date
	buffer[0] = 0x00;
	result = SD_read_conf (char_buff);
	if (result == 1) 	HAL_GPIO_WritePin(GPIOA, D3_Pin, GPIO_PIN_RESET);
	Take_Num_String(buffer, char_buff, BUFF_MAX_SIZE);
	if(buffer[0] == 0x01) { //If SET = 0 or no config file return
	
		sTime.Hours = buffer[1];
		sTime.Minutes = buffer[2];
		sTime.Seconds = buffer[3];
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;
		if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
		{
			Error_Handler();
		}
	 
		sDate.Date = buffer[4];
		sDate.Month = buffer[5];
		sDate.Year = buffer[6];
		sDate.WeekDay = buffer[7];

		if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
		{
			Error_Handler();
		}
		
		if(Change_SD_Set()) HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_RESET); // Changing set in config file after setting Time Date
	
}
	

	
}

//Alarm IT set --------------------------------------------------------------------------------------------
void Set_RTC_Alarm () {
	
//	RTC_DateTypeDef Date;
//	RTC_TimeTypeDef Time;
	uint8_t size = 0;
	RTC_AlarmTypeDef sAlarm;
	char buffer_sd [50];
	
	
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BCD);
	
	if (Time.Minutes < 0x30) {
		sAlarm.AlarmTime.Hours = Time.Hours;
		sAlarm.AlarmTime.Minutes = 0x30;
		sAlarm.AlarmDateWeekDay = Date.WeekDay;
	}
	else {
		sAlarm.AlarmTime.Minutes = 0x0;
		sAlarm.AlarmTime.Hours = Time.Hours+1;
		sAlarm.AlarmDateWeekDay = Date.WeekDay;
		if(Time.Hours == 0x09) sAlarm.AlarmTime.Hours = 0x10;
		else if(Time.Hours == 0x19) sAlarm.AlarmTime.Hours = 0x20;
		else if(Time.Hours == 0x23){
			sAlarm.AlarmTime.Hours = 0x00;
			if(Date.WeekDay < 0x07) sAlarm.AlarmDateWeekDay = Date.WeekDay+1;
			else sAlarm.AlarmDateWeekDay = 0x1;
		}

	}
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;	
  sAlarm.Alarm = RTC_ALARM_A;
	
	size = sprintf(buffer_sd, "Alarm: %x:%x, Wd: %x\r\n", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmDateWeekDay);
	if(SD_alarm(buffer_sd, size)) HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_RESET);
	
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
		
	size_lcd_alm = sprintf((char*)buffer_lcd_alm, "Next reg: %x:%x\nWeekday: %x", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes, sAlarm.AlarmDateWeekDay);
	
}



//Helping functions --------------------------------------------------------------------------------------------
// Delay --------------------------------------------------------------------------------------------
void Delay(uint16_t ms) {
	
	for(;ms==0;ms--) {
		__WFI();
	}
	//HAL_Delay(ms);
	
}

// Taking number bytes from string
uint8_t Take_Num_String(char * num_buff, char * char_buff, size_t size) { // max 255 bytes (uint8_t)
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
	static RTC_DateTypeDef Date;
	static RTC_TimeTypeDef Time;
	static short int num_of_meas = 0;
	
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BCD);
	
	size = sprintf(buffer, "%x %x %x %x %x  %.2f %.2f\r\n", Date.Date, Date.Month, Date.Year, Time.Hours, Time.Minutes, press, temp);

	if(SD_add_reg(buffer, size)) HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_RESET);
	else num_of_meas++;
	
	size_lcd_reg = sprintf((char*)buffer_lcd_reg, "Last reg: %x:%x\nMeas saved: %i", Time.Hours, Time.Minutes, num_of_meas);
	
}


//Getting Time
void Lcd_Get_Time () {
//	static RTC_DateTypeDef Date;
//	static RTC_TimeTypeDef Time;
	static short int sec = 88;
	
	if(sec > 59) {
		HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BCD);
		HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BCD);
		sec = Time.Seconds;
	}
	size_lcd_time = sprintf((char*)buffer_lcd_time, "Time:%x:%x:%i\nDate:%x.%x.%x %x", Time.Hours, Time.Minutes, sec, Date.Date, Date.Month, Date.Year, Date.WeekDay);
	sec++;
}

void Get_Meas(float *temp, float *press) {
	static float last_temp = 0;
	static float last_press = 0;
	
	if (!bmp280_read_float(&bmp280, temp, press, &humidity)) size_lcd_meas = sprintf((char *)buffer_lcd_meas, "reading fail");
	
	
	//reset czujnika jesli nie wysyla danych
	if(*press == last_press) {
		
		Delay(3000);
		NVIC_SystemReset();
		
	}
	else *press /=100;
	
	size_lcd_meas = sprintf((char *)buffer_lcd_meas,"Press:%.2fhPa\nTemp:%.2f*C", *press, *temp);
	buffer_lcd_meas[size_lcd_meas-2] = 0x00;
	last_temp = *temp;
	last_press = *press;

}

// Init function (before loop)
void setup (void) {
	HAL_GPIO_WritePin(GPIOA, D3_Pin, GPIO_PIN_SET);  //Turn off LEDs
	HAL_GPIO_WritePin(GPIOA, D2_Pin, GPIO_PIN_SET);
	
	RTC_init_SD();
	lcdInit(&hi2c2, 0x3F, 2, 16);
	Set_RTC_Alarm ();
	
	lcdLoadCustomChar(0, stopien);
	
	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
	bmp280.i2c = &hi2c1;

	while (!bmp280_init(&bmp280, &bmp280.params)) {
		size_lcd_meas = sprintf((char *)buffer_lcd_meas, "BMP280 init fail");
		lcdDisplayClear();
		lcdPrintStr(buffer_lcd_meas, size_lcd_meas);
		Delay(2000);
	}
	size_lcd_meas = sprintf((char *)buffer_lcd_meas, "BMP280 found ");
	lcdDisplayClear();
	lcdPrintStr(buffer_lcd_meas, size_lcd_meas);

}




//main loop
void loop (void) {
	

	if (Flaga5ms) {
		Flaga5ms = 0;
		
		if(btn_repeat_save > 0) btn_repeat_save++;
		if(btn_repeat_save > 50) btn_repeat_save = 0;
	}
	
		if (Flaga250ms) {
		Flaga250ms = 0;
								
	}
		
	if (Flaga500ms) {
		Flaga500ms = 0;
					
		lcdDisplayClear();
		switch (display_mode) {
			case 0:
				lcdPrintStr(buffer_lcd_error, size_lcd_error);
				break;
			case 1:
				lcdPrintStr(buffer_lcd_meas, size_lcd_meas);
				break;
			case 2:
				lcdPrintStr(buffer_lcd_time, size_lcd_time);
				break;
			case 3:
				lcdPrintStr(buffer_lcd_reg, size_lcd_reg);
				break;
			case 4:
				lcdPrintStr(buffer_lcd_alm, size_lcd_alm);
				break;
		}
	}
	
	if (Flaga1000ms) {
		Flaga1000ms = 0;

		Get_Meas(&temperature, &pressure);

		Lcd_Get_Time();
	}
	
	__WFI();
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
	static uint8_t lcd_energy = 0;
	
	if(btn_repeat_save == 0) {
		
		
		if(GPIO_Pin == BTN1_Pin) {
			display_mode++;
			if(display_mode > 4) display_mode = 0;

		}
		
		if (GPIO_Pin == BTN2_Pin){
			
			switch (lcd_energy){
				case 0: 
					lcdBacklightOff();
				break;				
				case 1:
					lcdDisplayOff();
				break;
				case 2:
					lcdDisplayOn();
					lcdBacklightOn();
				break;
				}
			lcd_energy++;
			if(lcd_energy > 2) lcd_energy = 0;
				
		}
		
		if (GPIO_Pin == BTN3_Pin){
			Meas_Save(pressure, temperature);
			Set_RTC_Alarm ();
		}
		
		if (GPIO_Pin == BTN4_Pin){
			
			NVIC_SystemReset();
			__NOP();

		}
		btn_repeat_save = 1;
	}
	
}




void RTC_Alarm_Handler() {
	Meas_Save(pressure, temperature);
	Set_RTC_Alarm ();
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



