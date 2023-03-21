#include "commsecDecodeState_monitor.h"
#include <smaccm_types.h>
#include <stdio.h>

bool validkey = false;

struct gec_sym_key ctx_dl_global_gec_sym_key_dec;

void callback_gec_decode_init(const uint8_t n_var0[GEC_RAW_KEY_LEN])
{
    ctx_dl_decode_init(n_var0);
    validkey = true;
}

void callback_cyphertext_decode(const uint8_t n_var0[96U])
{
    bool n_deref0 = validkey;
    
    if (n_deref0) {
        // uint8_t n_local1[80U] = {};
        SMACCM_DATA__GIDL_container n_local1;
        // uint8_t *n_ref2 = n_local1;
        SMACCM_DATA__GIDL_container *n_ref2 = &n_local1;
        uint32_t n_r3 = ctx_dl_decode_run(n_var0, n_ref2);
        
        if (!n_r3) {
            callback_output_self2server_enqueue(n_ref2);
        } else {
            printf("ctx_dl_decode_run failed\n");
        }
    } else {
        printf("%s: No valid key\n", __FUNCTION__);
    }
}

void ctx_dl_decode_init(const uint8_t n_var0[24U])
{
    gec_init_sym_key_conf_auth(&ctx_dl_global_gec_sym_key_dec, n_var0);
}

uint32_t ctx_dl_decode_run(const uint8_t n_var0[96U], SMACCM_DATA__GIDL_container *n_var1)
{
    uint32_t n_r0 = gec_decrypt(&ctx_dl_global_gec_sym_key_dec, n_var0, n_var1);
    
    return n_r0;
}
