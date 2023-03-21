#ifndef TB_VERIFY
#include <stdio.h>
#endif // TB_VERIFY
#include "../../../../include/smaccm_types.h"
#include "../include/Server_decrypt2self_Monitor.h"

int mon_get_sender_id(void);
int monsig_emit(void);

SMACCM_DATA__GIDL_container contents[1];
static uint32_t front = 0;
static uint32_t length = 0;

static bool is_full(void) {
  return length == 1;
}

static bool is_empty(void) {
  return length == 0;
}
bool mon_dequeue(SMACCM_DATA__GIDL_container * m) {
  if (mon_get_sender_id() != MONITOR_READ_ACCESS) {
    #ifndef TB_VERIFY
    // #ifdef CONFIG_APP_SMACCMPILOT_TK1_TB_DEBUG
    fprintf(stderr, "Monitor Server_decrypt2self: attempt to dequeue without permission\n");
    // #endif // CONFIG_APP_SMACCMPILOT_TK1_TB_DEBUG
    #endif // !TB_VERIFY
    return false;
  } else if (is_empty()) {
    return false;
  } else {
    *m = contents[front];
    front = (front + 1) % 1;
    length--;
    return true;
  }
}

bool mon_enqueue(const SMACCM_DATA__GIDL_container * m) {
  if (mon_get_sender_id() != MONITOR_WRITE_ACCESS) {
    #ifndef TB_VERIFY
    // #ifdef CONFIG_APP_SMACCMPILOT_TK1_TB_DEBUG
    fprintf(stderr, "Monitor Server_decrypt2self: attempt to enqueue without permission\n");
    // #endif // CONFIG_APP_SMACCMPILOT_TK1_TB_DEBUG
    #endif // !TB_VERIFY
    return false;
  } else if (is_full()) {
    // #ifdef CONFIG_APP_SMACCMPILOT_TK1_TB_DEBUG
    fprintf(stderr,"Monitor Server_decrypt2self is full!\n");
    // #endif 
    return false;
  } else {
    contents[(front + length) % 1] = *m;
    length++;
    monsig_emit();
    return true;
  }
}
