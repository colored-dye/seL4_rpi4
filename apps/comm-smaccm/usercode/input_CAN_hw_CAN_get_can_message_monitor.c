/* This file has been autogenerated by Ivory
 * Compiler version  0.1.0.3
 */
#include "input_CAN_hw_CAN_get_can_message_monitor.h"

void callback_input_CAN_hw_CAN_get_can_message_handler(const struct can_message *n_var0)
{
    callback_receive_msg(n_var0);
}