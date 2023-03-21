#ifndef __TB_AADL_smaccmpilot_tk1_types__H
#define __TB_AADL_smaccmpilot_tk1_types__H

#include <stdbool.h>
#include <stdint.h>
#ifndef TB_VERIFY
#include <stddef.h>
#endif // !TB_VERIFY

#define __TB_OS_CAMKES__
#define MONITOR_READ_ACCESS 111
#define MONITOR_WRITE_ACCESS 222

// Wrapper for mutex/callback calls
// Show error messages and then abort
#ifndef TB_VERIFY
#define MUTEXOP(OP)\
if((OP) != 0) {\
  fprintf(stderr,"Operation " #OP " failed in %s at %d.\n",__FILE__,__LINE__);\
  *((int*)0)=0xdeadbeef;\
}
#else 
#define MUTEXOP(OP) OP
#endif // TB_VERIFY

#ifndef TB_VERIFY
#define CALLBACKOP(OP)\
if((OP) != 0) {\
  fprintf(stderr,"Operation " #OP " failed in %s at %d.\n",__FILE__,__LINE__);\
  *((int*)0)=0xdeadbeef;\
}
#else 
#define CALLBACKOP(OP) OP
#endif // TB_VERIFY

typedef uint8_t SMACCM_DATA__CAN_Buffer [8]; 

typedef uint8_t SMACCM_DATA__UART_Buffer [255]; 

typedef 
      struct SMACCM_DATA__UART_Packet_i { 
         SMACCM_DATA__UART_Buffer buf  ; 
         int32_t buf_len  ; 
      } SMACCM_DATA__UART_Packet_i ; 

typedef uint8_t SMACCM_DATA__GIDL [80]; 

typedef 
      struct SMACCM_DATA__Camera_Bounding_Box_i { 
         uint16_t left  ; 
         uint16_t right  ; 
         uint16_t top  ; 
         uint16_t bottom  ; 
      } SMACCM_DATA__Camera_Bounding_Box_i ; 

typedef 
      struct SMACCM_DATA__CAN_Frame_i { 
         uint32_t id  ; 
         SMACCM_DATA__CAN_Buffer buf  ; 
         uint8_t buf_len  ; 
      } SMACCM_DATA__CAN_Frame_i ; 

typedef 
      struct tb_SMACCM_DATA__CAN_Buffer_container { 
         SMACCM_DATA__CAN_Buffer f  ; 
      } tb_SMACCM_DATA__CAN_Buffer_container ; 

typedef 
      struct tb_SMACCM_DATA__UART_Buffer_container { 
         SMACCM_DATA__UART_Buffer f  ; 
      } tb_SMACCM_DATA__UART_Buffer_container ; 

typedef 
      struct SMACCM_DATA__GIDL_container { 
         SMACCM_DATA__GIDL f  ; 
      } SMACCM_DATA__GIDL_container ; 

#endif // __TB_AADL_smaccmpilot_tk1_types__H
