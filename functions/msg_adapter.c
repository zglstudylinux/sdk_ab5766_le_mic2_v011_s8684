#include "include.h"
#include "func.h"


AT(.text.func.adapter)
void func_adapter_message(u16 msg)
{
    switch (msg) {
//    case KU_PLAY:
//    case KU_PLAY_USER_DEF:
//    case KU_PLAY_PWR_USER_DEF:
//        break;
//
//    case KL_PLAY_PWR_USER_DEF:
//    case KL_PLAY_USER_DEF:
//        break;
#if ADAPTER_USB_MIC_RX_EN
    case EVT_PC_INSERT:
        printf("EVT_PC_INSERT\n");
        usb_device_enter(UDE_ENUM_TYPE);
        adapter_usb_init_flag = 1;
        break;

    case EVT_PC_REMOVE:
        printf("EVT_PC_REMOVE\n");
        usb_device_exit();
        adapter_usb_init_flag = 0;
        break;
//
//    case EVT_USB_SYNC_VOL:
////        my_printf("EVT_USB_SYNC_VOL\n");
//        usb_connected_sync_volume();
//        break;
#endif
#if ADAPTER_UART_COMMAND_EN
    case EVT_UART_COMMAND_PROC:
        uart_command_rx_proc();
        break;
#endif

    case MSG_SYS_1S:
#if WIRELESS_MIC_DUMP_PER_BER
        wireless_mic_dump();
#endif
        break;
    default:
        func_message(msg);
        break;
    }
}
