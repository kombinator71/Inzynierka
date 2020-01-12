
#define U_START_FLOW ((uint8_t)0x01)
#define U_END_FLOW ((uint8_t) 0xFE)

#define U_SEND_MEAS ((uint8_t)0x02)
#define U_MEAS_RECIEVED 0x15
#define U_ERROR ((uint8_t) 0xFF)
#define U_CONNECTED ((uint8_t)0x11)

#define COMM_SIZE 1 //command size
#define MEAS_SIZE 17

typedef enum {
	OK = 0,
	RES_UART_ERROR = 1,
	RES_UART_MEAS_ERROR = 2
} result_t;

