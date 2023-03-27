#pragma once

#include "utils/util.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

/*
 * Queue
 */
#define MAX_QUEUE_SIZE 2048
typedef struct queue {
    uint8_t raw_queue[MAX_QUEUE_SIZE];
    uint32_t head;
    uint32_t size;
} queue_t;

void queue_init(queue_t *q) {
    memset(q->raw_queue, -1, MAX_QUEUE_SIZE);
    q->head = 0;
    q->size = 0;
}

inline int queue_full(queue_t *q) {
    return q->size == MAX_QUEUE_SIZE;
}

inline int queue_empty(queue_t *q) {
    return !q->size;
}

inline int enqueue(queue_t *q, uint8_t x) {
    if (queue_full(q)) {
        LOG_ERROR("Cannot enquque");
        return -1;
    }
    int index = (q->head + q->size) % MAX_QUEUE_SIZE;
    q->raw_queue[index] = x;
    q->size++;
    return 0;
}

inline int dequeue(queue_t *q, uint8_t *ret) {
    if (queue_empty(q)) {
        LOG_ERROR("Cannot dequeue");
        return -1;
    }

    *ret = q->raw_queue[q->head];
    q->head = (q->head + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return 0;
}

inline void print_queue(queue_t *q) {
    char str[MAX_QUEUE_SIZE * 3 + 1];
    char *p = str;
    LOG_ERROR("Print queue:");
    int size = q->size;
    for (int h = q->head, i = 0; i < size; i++, h = (h+1) % MAX_QUEUE_SIZE) {
        sprintf(p, "%02X ", q->raw_queue[h]);
        p += 3;
    }
    LOG_ERROR("%s", str);
}
