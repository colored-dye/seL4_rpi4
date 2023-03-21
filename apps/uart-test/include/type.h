#include <stdint.h>

typedef struct UART_Mem {
    uint8_t buf[0x200];
} UART_Mem;