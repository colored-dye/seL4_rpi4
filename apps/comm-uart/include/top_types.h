#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utils/util.h>

typedef uint8_t FC_Data_raw [256];
typedef struct FC_Data {
    FC_Data_raw raw_data;
    uint32_t len;
} FC_Data;

typedef uint8_t Telem_Data_raw [256];
typedef struct Telem_Data {
    Telem_Data_raw raw_data;
    uint32_t len;
} Telem_Data;

#define serial_printf(...) do { \
    char __str[256]; \
    sprintf(__str, __VA_ARGS__); \
    for (int i=0; i<sizeof(__str) && __str[i]; i++) { \
        ps_cdev_putchar(serial, __str[i]); \
    } \
} while (0)

/*
 * Queue
 */
#define MAX_QUEUE_SIZE 1024
typedef struct queue {
    uint8_t raw_queue[MAX_QUEUE_SIZE];
    uint32_t head;
    uint32_t size;
} queue_t;

#define queue_init(q) \
    ({ \
        memset((q)->raw_queue, -1, MAX_QUEUE_SIZE); \
        (q)->head = 0; \
        (q)->size = 0; \
    })

#define queue_full(q) \
    ({ \
        (q)->size == MAX_QUEUE_SIZE; \
    })

#define queue_empty(q) \
    ({ \
        !(q)->size; \
    })

#define enqueue(q, x) \
    ({ \
        int _ret; \
        if (queue_full(q)) { \
            LOG_ERROR("Cannot enquque"); \
            _ret = -1; \
        } else {\
        int _index = ((q)->head + (q)->size) % MAX_QUEUE_SIZE; \
        (q)->raw_queue[_index] = x; \
        (q)->size++; \
        _ret = 0; \
        } \
        _ret; \
    })

#define dequeue(q, ret) \
    ({ \
        int _ret; \
        if (queue_empty(q)) { \
            LOG_ERROR("Cannot dequeue"); \
            _ret = -1; \
        } else { \
        *(ret) = (q)->raw_queue[(q)->head]; \
        (q)->head = ((q)->head + 1) % MAX_QUEUE_SIZE; \
        (q)->size--; \
        _ret = 0; \
        } \
        _ret; \
    })

#define print_queue(q) \
    ({ \
        char _str[MAX_QUEUE_SIZE * 3 + 1]; \
        char *_p = _str; \
        LOG_ERROR("Print queue:"); \
        int size = (q)->size; \
        for (int h = (q)->head, i = 0; i < size; i++, h = (h+1) % MAX_QUEUE_SIZE) { \
            sprintf(_p, "%02X ", (q)->raw_queue[h]); \
            _p += 3; \
        } \
        LOG_ERROR("%s", _str); \
    })

#define print_serial(s) { \
    for (int i=0; s[i]; i++) { \
        ps_cdev_putchar(serial, s[i]); \
    } \
}

#define print_queue_serial(q) \
    ({ \
        char _str[MAX_QUEUE_SIZE * 3 + 1]; \
        char *_p = _str; \
        const char *__prompt = "Print queue:"; \
        print_serial(__prompt); \
        int size = (q)->size; \
        for (int h = (q)->head, i = 0; i < size; i++, h = (h+1) % MAX_QUEUE_SIZE) { \
            sprintf(_p, "%02X ", (q)->raw_queue[h]); \
            _p += 3; \
        } \
        print_serial(_str); \
    })
