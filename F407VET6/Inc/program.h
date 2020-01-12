/**
 * Wojciech Dron
 * Praca dyplomowa inzynierska
 * Mikroprocesorowy Rejestrator Cisnienia Atmosferycznego 2019
*/

#include "stm32f4xx_hal.h"


static uint8_t Flaga5ms;
static uint8_t Flaga250ms;
static uint8_t Flaga500ms;
static uint8_t Flaga1000ms;

uint8_t Take_Num_String(char * num_buff, char * char_buff, size_t size);
void Delay(uint16_t ms);

void loop (void);
void setup (void);

void RTC_Alarm_Handler(void);
void SysTick_Flags(void);
void BTN_Callback (uint16_t GPIO_Pin);

