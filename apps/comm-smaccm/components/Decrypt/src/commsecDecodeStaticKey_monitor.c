#include "commsecDecodeStaticKey_monitor.h"
#include <stdio.h>

void callback_configureStaticKey(const int64_t *n_var0) {
    (void)n_var0; // UNUSED
    uint8_t n_local0[24] = {24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint8_t *n_ref1 = n_local0;
    
    callback_gec_decode_init(n_ref1);

    printf("Static Key: ");
    for (int i=0; i<24; i++) {
        printf("%X ", n_local0[i]);
    }
    putchar('\n');
}
