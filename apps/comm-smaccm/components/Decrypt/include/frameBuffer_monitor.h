#ifndef __FRAMEBUFFER_MONITOR_H__
#define __FRAMEBUFFER_MONITOR_H__

// #ifdef __cplusplus
// extern "C" {
// #endif

#include <smaccm_types.h>

#define FRAMEBUFFER_NUM 4
#define FRAMEBUFFER_SIZE 96

extern int32_t frameBuffer_2_ringbuffer_insert;
extern int32_t frameBuffer_2_ringbuffer_remove;
extern uint8_t frameBuffer_2_ringbuffer_buf[FRAMEBUFFER_NUM][FRAMEBUFFER_SIZE];

void callback_push(const uint8_t n_var0[FRAMEBUFFER_SIZE]);
void callback_periodic_pop(const int64_t *n_var0);
bool frameBuffer_2_ringbuffer_push(const uint8_t n_var0[FRAMEBUFFER_SIZE]);
bool frameBuffer_2_ringbuffer_pop(uint8_t n_var0[FRAMEBUFFER_SIZE]);

// #ifdef __cplusplus
// }
// #endif

#endif
