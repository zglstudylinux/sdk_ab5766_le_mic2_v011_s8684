#include "include.h"
#include "func.h"
#include "func_adapter.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

enum {
    ADAPTER_STA_INIT_IDLE,
    ADAPTER_STA_INIT_W4_CONNECT,
    ADAPTER_STA_START_ACTION,
    ADAPTER_STA_IDLE,
};

enum {
    DEV_USBPC = 0,
    DEV_TOTAL_NUM,
};

static struct {
    uint8_t init_flag;
    uint8_t init_state;
    uint32_t ticks;
} adapter;

#if ADAPTER_USB_MIC_RX_EN
volatile uint8_t adapter_usb_init_flag = 0;

AT(.usbdev.com.detect)
void usb_detect(void)
{
    u8 usb_sta = usbchk_connect();
    if (usb_sta == 1) {
        if (dev_online_filter(DEV_USBPC)) {
            msg_enqueue(EVT_PC_INSERT);
//            printf("pc insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_USBPC)) {
            msg_enqueue(EVT_PC_REMOVE);
            pc_remove();
//            printf("pc remove\n");
        }
    }

}
#endif


AT(.text.func.adapter)
void func_adapter_init(void)
{
//    //放到连接后再初始化，节省功耗
//    if (!adapter.init_flag) {
//        adapter.init_flag = 1;
//        adapter_init();
//    }
    adapter.init_state = ADAPTER_STA_INIT_IDLE;
#if BSP_LED_EN
    led_bt_idle();
#endif
}

AT(.text.func.mic_emit)
static void func_adapter_process_do(void)
{
    u8 link_nb;

    if(wireless_mic.change_flag) {
        wireless_mic.change_flag = 0;
        adapter.init_state = ADAPTER_STA_START_ACTION;
    }

    switch(adapter.init_state) {
    case ADAPTER_STA_INIT_IDLE:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, init(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_mic.connected_sta, link_nb);

        if(link_nb != 0) {
            //有回连信息，等待被连接（关闭可被发现）
            ble_adv_set_enable(1, 0);
            adapter.init_state  = ADAPTER_STA_INIT_W4_CONNECT;
            adapter.ticks       = tick_get();
        } else {
            //没有回连信息，等待被发现和连接
            ble_adv_set_enable(1, 1);
            adapter.init_state  = ADAPTER_STA_IDLE;
        }
        break;

    case ADAPTER_STA_INIT_W4_CONNECT:
        if(tick_check_expire(adapter.ticks, 2000)) {
            adapter.init_state = ADAPTER_STA_START_ACTION;
        }
        break;

    case ADAPTER_STA_START_ACTION:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, con_sta(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_mic.connected_sta, link_nb);

        if(wireless_mic.connected_sta == WIRELESS_MIC_STA_MASK) {
            //两个都连上了，关闭可被发现和可被连接
            ble_adv_set_enable(0, 0);
        } else if(wireless_mic_is_bonding() && link_nb >= WIRELESS_CON_LINK_NB) {
            //组队绑定时，切配对过两个，等待被连接（关闭可被发现）
            ble_adv_set_enable(1, 0);
        } else {
            //等待被发现和连接
            ble_adv_set_enable(1, 1);
        }
        adapter.init_state = ADAPTER_STA_IDLE;
        break;
    }
}

#if WIRELESS_CON_PAIR_MODE
extern uint8_t cfg_wireless_con_id;
void ble_set_con_id(uint param)
{
    ble_adv_set_enable(0, 0);
    cfg_wireless_con_id = param;
    btstack_ble_reset_hash(2);
    ble_adv_set_enable(1, 1);
}
AT(.text.bsp.wireless_cb)
void wireless_con_role(void)          //判断主副麦
{
    if(!wireless_mic_connected_sta_get()) {
        if(tick_check_expire(wireless_mic.con_id_switch_ticks, 3000)){     //3s切换一次广播数据包
            wireless_mic.con_id_switch_ticks = tick_get();
            if(wireless_mic.con_role_switch_flag) {
                ble_set_con_id(ADAPTER_SET_TX1_CON_ID);
                sys_cb.con_role = 1;
                wireless_mic.con_role_switch_flag = 0;
                printf("Master ADV\n");
            } else {
                ble_set_con_id(ADAPTER_SET_TX2_CON_ID);
                sys_cb.con_role = 0;
                wireless_mic.con_role_switch_flag = 1;
                printf("Slave ADV\n");
            }
        }
    } else if((wireless_mic_connected_sta_get() != 3) && !wireless_mic.con_temp) {
        wireless_mic.con_temp = 1;
        if(sys_cb.con_role) {           //已经连上主麦
            ble_set_con_id(ADAPTER_SET_TX2_CON_ID);
            sys_cb.con_role = 0;
            printf("Slave ADV\n");
        } else {                       //已经连上副麦
            ble_set_con_id(ADAPTER_SET_TX1_CON_ID);
            sys_cb.con_role = 1;
            printf("Master ADV\n");
        }
    }
}
#endif

AT(.text.func.process.adapter)
void func_adapter_process(void)
{
#if TRACE_EN
    static u8 sta = 0xff;
    if(sta != adapter.init_state) {
        sta = adapter.init_state;
        TRACE("adapter, state: %d\n", sta);
    }
#endif
#if WIRELESS_CON_PAIR_MODE
    if(!wireless_mic.con_id_switch_ticks) {
        wireless_mic.con_id_switch_ticks = tick_get();
        printf("con_role_init\n");
        ble_set_con_id(0);
    }

    wireless_con_role();
#endif
    if(adapter.init_state != ADAPTER_STA_IDLE || wireless_mic.change_flag) {
        func_adapter_process_do();
    }

    if(wireless_mic.connected_sta != sys_cb.disp_sta) {
        wireless_mic_proc();
    }
    func_process();
#if ADAPTER_USB_MIC_RX_EN
    if (xcfg_cb.wireless_adapter_en) {
        usb_detect();
    }

    if (adapter_usb_init_flag) {
        usb_device_process();
    }
#endif
    wireless_dump_proc();
}

AT(.text.func.adapter)
static void func_adapter_enter(void)
{
    msg_queue_clear();
#if ADAPTER_UART_COMMAND_EN
    uart_command_init();
#endif
    func_adapter_init();
    bsp_ble_init();
}

AT(.text.func.adapter)
static void func_adapter_exit(void)
{
    bt_off();
    func_cb.last = FUNC_ADAPTER;
}

AT(.text.func.adapter)
void func_adapter(void)
{
    printf("%s\n", __func__);

    func_adapter_enter();

    while (func_cb.sta == FUNC_ADAPTER) {
        func_adapter_process();
        u16 msg = msg_dequeue();
        if (msg != MSG_NO) {
            func_adapter_message(msg);
        }
        //func_wirelsee_mic_emit_display();
    }

    func_adapter_exit();
}

