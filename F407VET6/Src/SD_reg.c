/**
 * Wojciech Dron
 * Praca dyplomowa inzynierska
 * Mikroprocesorowy Rejestrator Cisnienia Atmosferycznego 2019
*/


#include "stm32f4xx_hal.h"
#include "main.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "fatfs.h"
#include "program.h"
#include "SD_reg.h"


/* Private variables ---------------------------------------------------------*/
FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL FileReg;     /* File object */
char FileRegName[] = "reg.txt"; //Change filename

FIL FileConf;     /* File object */
char FileConfName[] = "config.txt"; //Change filename

FIL FileAlm;     /* File object */
char FileAlmName[] = "alarms.txt"; //Change filename

UINT byte;
extern char SDPath[4]; /* SD card logical drive path */



//uint8_t SD_add_reg (uint8_t * sd_buffer, uint16_t size) {
uint8_t SD_read_conf (char * buffer) {
	UINT size = BUFF_MAX_SIZE;

	if(f_mount(&SDFatFs, SDPath, 0) != FR_OK) return 1; // Return error
	if(f_open (&FileConf, FileConfName,  FA_READ) != FR_OK) return 1; // Empty SD Slot or wrong filename
	if(f_read (&FileConf, buffer, size, &size) != FR_OK) return 1; // Empty SD Slot or wrong filename
	f_close(&FileConf);
	
	return 0;
}

uint8_t Change_SD_Set () {
	if(f_mount(&SDFatFs, SDPath, 0) != FR_OK) return 1; // Return error
	if(f_open (&FileConf, FileConfName,  FA_WRITE) != FR_OK) return 1; // Empty SD Slot or wrong filename
	if(f_write(&FileConf, "00", 2, &byte) != FR_OK) return 1;
	f_close(&FileConf);
	
	return 0;
}

uint8_t SD_add_reg (char * sd_buffer, uint16_t size) {	

	if(f_mount(&SDFatFs, SDPath, 0) != FR_OK) return 1; // Return error
	if(f_open (&FileReg, FileRegName,  FA_OPEN_APPEND | FA_WRITE) != FR_OK) return 1; // Empty SD Slot or wrong filename
	if(f_lseek(&FileReg, f_size(&FileReg)) != FR_OK) return 1;
	if(f_write(&FileReg, sd_buffer, size, &byte) != FR_OK) return 1;
	f_close(&FileReg);
	
	
	return 0;
}

uint8_t SD_alarm (char * sd_buffer, uint16_t size) {	

	if(f_mount(&SDFatFs, SDPath, 0) != FR_OK) return 1; // Return error
	if(f_open (&FileAlm, FileAlmName,  FA_OPEN_APPEND | FA_WRITE) != FR_OK) return 1; // Empty SD Slot or wrong filename
	if(f_lseek(&FileAlm, f_size(&FileReg)) != FR_OK) return 1;
	if(f_write(&FileAlm, sd_buffer, size, &byte) != FR_OK) return 1;
	f_close(&FileAlm);
	
	
	return 0;
}


