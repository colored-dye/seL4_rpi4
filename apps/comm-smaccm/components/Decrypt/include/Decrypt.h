#ifndef __DECRYPT_H__
#define __DECRYPT_H__

#include <smaccm_types.h>
#include "commsecDecodeStaticKey_monitor.h"
#include "frameBuffer_monitor.h"

void component_entry(const int64_t * periodic_dispatcher);
void component_init(const int64_t *arg);
bool self2server_enqueue(const SMACCM_DATA__GIDL_container *self2server);

#endif
