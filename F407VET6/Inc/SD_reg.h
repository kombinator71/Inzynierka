/**
 * Wojciech Dron
 * Praca dyplomowa inzynierska
 * Mikroprocesorowy Rejestrator Cisnienia Atmosferycznego 2019
*/

#define BUFF_MAX_SIZE 70


uint8_t SD_read_conf(char * buffer);
uint8_t SD_add_reg (char * sd_buffer, uint16_t size);
uint8_t Change_SD_Set (void);
uint8_t SD_alarm (char * sd_buffer, uint16_t size);
