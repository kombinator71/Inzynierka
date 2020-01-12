#include "stm32l0xx_hal.h"

void i2c_scan_addr_uart (I2C_HandleTypeDef * hi2c, UART_HandleTypeDef * huart) {

	uint8_t text_uart [32];
	HAL_StatusTypeDef result;
 	uint8_t i;
	text_uart [0] = 170;
	text_uart [1] = 0;
	text_uart [2] = 170;

 	for (i=1; i<128; i++)
 	{
 	  /*
 	   * the HAL wants a left aligned i2c address
 	   * &hi2c1 is the handle
 	   * (uint16_t)(i<<1) is the i2c address left aligned
 	   * retries 2
 	   * timeout 2
 	   */
 	  result = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 2, 2);
 	  if (result != HAL_OK) // HAL_ERROR or HAL_BUSY or HAL_TIMEOUT
 	  {
 		 // printf("."); // No ACK received at that address
 	  }
 	  if (result == HAL_OK)
 	  {
			text_uart [0] = i;
 		  HAL_UART_Transmit(huart, text_uart , 1, 1000); 
 	  }
 	}
	

	HAL_UART_Transmit(huart, text_uart+1 , 2, 1000); 

}
