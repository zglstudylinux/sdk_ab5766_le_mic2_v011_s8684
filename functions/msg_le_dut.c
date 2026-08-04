#include "include.h"

AT(.text.func.le_dut.msg)
void func_le_dut_message(uint16_t msg)
{
    switch (msg) {
        case MSG_SYS_1S:
            //printf("MSG_SYS_1S\n");
            break;

        default:
            func_message(msg);
            break;
    }
}
