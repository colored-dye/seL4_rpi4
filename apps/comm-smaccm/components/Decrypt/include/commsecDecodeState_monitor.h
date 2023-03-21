#ifndef __COMMSECDECODESTATE_MONITOR_H__
#define __COMMSECDECODESTATE_MONITOR_H__
// #ifdef __cplusplus
// extern "C" {
// #endif
#include "ivory.h"
// #include "smaccm_UART_module.h"
// #include "towerUartDeps.h"
// #include "hxstream_state_module.h"
#include "ivory_serialize.h"
#include "output_self2server_enqueue_monitor.h"
#include "gec.h"
#include <smaccm_types.h>
extern bool validkey;
extern struct gec_sym_key ctx_dl_global_gec_sym_key_dec;
void callback_gec_decode_init(const uint8_t n_var0[24U]);
void callback_cyphertext_decode(const uint8_t n_var0[96U]);
void ctx_dl_decode_init(const uint8_t n_var0[24U]);
uint32_t ctx_dl_decode_run(const uint8_t n_var0[96U], SMACCM_DATA__GIDL_container *n_var1);

// #ifdef __cplusplus
// }
// #endif
#endif /* __COMMSECDECODESTATE_MONITOR_H__ */
