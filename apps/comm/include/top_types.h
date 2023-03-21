#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t FC_Data_raw [80];
typedef struct FC_Data {
    FC_Data_raw raw_data;
} FC_Data;

typedef uint8_t UART_Data_raw [255];
typedef struct UART_Data {
    UART_Data_raw raw_data;
} UART_Data;
