#include "include.h"
#include "func.h"
#include "func_mic_emit.h"
#include "driver_gpio.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN
void mic_mix_process(void);

enum {
    MIC_EMIT_STA_INIT_IDLE,
    MIC_EMIT_STA_INIT_CONNECT,
    MIC_EMIT_STA_START_ACTION,
    MIC_EMIT_STA_DELAY,

    MIC_EMIT_STA_IDLE,
    MIC_EMIT_STA_SCAN,
};

static struct {
    uint8_t init_flag;
    uint8_t init_state;
    uint8_t con_state;
    uint32_t ticks;
    uint tick_delay;
} mic_emit AT(.buf.mic_emit.proc);


AT(.text.func.mic_emit)
void func_mic_emit_init(void)
{
//    //放到连接后再初始化，节省功耗
//    if (!mic_emit.init_flag) {
//        mic_emit.init_flag = 1;
//        mic_emit_init();
//    }

    mic_emit.init_state = MIC_EMIT_STA_INIT_IDLE;
#if BSP_LED_EN
    led_bt_idle();
#endif
}

AT(.text.func.mic_emit)
static void func_mic_emit_process_do(void)
{
    u8 addr[6];

    if(wireless_mic.change_flag) {
        wireless_mic.change_flag = 0;

        switch(wireless_mic.change_sta[0]) {
        case 0:     //connect success
            mic_emit.init_state = MIC_EMIT_STA_START_ACTION;
            break;

        case 1:     //connect fail
            if(!wireless_mic_is_bonding()) {
                mic_emit.ticks      = tick_get();
                mic_emit.tick_delay = 100+bt_get_rand()%500;
                mic_emit.init_state = MIC_EMIT_STA_DELAY;
                break;
            }
            //no break

        case 2:     //disconnect
            mic_emit.ticks      = tick_get();
            mic_emit.tick_delay = 500+bt_get_rand()%500;
            mic_emit.init_state = MIC_EMIT_STA_DELAY;
            break;
        }
    }

    switch(mic_emit.init_state) {
    case MIC_EMIT_STA_INIT_IDLE:
        TRACE("emit, init(%d)\n", wireless_mic_is_bonding());
        if(bt_get_link_info_addr(addr, 0)) {
            //有回连信息，开始回连
            TRACE("emit, con_req: ");
            TRACE_R(addr, 6);
            ble_create_con_for_addr(addr, 1000);
            mic_emit.init_state  = MIC_EMIT_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("emit, scan_en\n");
            ble_scan_set_enable(1);
            mic_emit.init_state  = MIC_EMIT_STA_SCAN;
            mic_emit.ticks       = tick_get();
        }
        break;

    case MIC_EMIT_STA_INIT_CONNECT:
        break;

    case MIC_EMIT_STA_START_ACTION:
        TRACE("emit, con_sta(%d): %x\n", wireless_mic_is_bonding(), wireless_mic.connected_sta);

        if(wireless_mic.connected_sta) {
            //已连接，关闭扫描
            TRACE("emit, con_complete\n");
            mic_emit.init_state = MIC_EMIT_STA_IDLE;
        } else if(wireless_mic_is_bonding() && bt_get_link_info_addr(addr, 0)) {
            //有回连信息，开始回连
            TRACE("emit, con_req: ");
            TRACE_R(addr, 6);
            ble_create_con_for_addr(addr, 1000);
            mic_emit.init_state  = MIC_EMIT_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("emit, scan_en\n");
            ble_scan_set_enable(1);
            mic_emit.init_state = MIC_EMIT_STA_SCAN;
            mic_emit.ticks      = tick_get();
        }
        break;

    case MIC_EMIT_STA_SCAN:
        if(tick_check_expire(mic_emit.ticks, 500)) {     //省电时可以间歇性扫描
            ble_scan_set_enable(0);
            mic_emit.ticks      = tick_get();
//            mic_emit.tick_delay = 1000+bt_get_rand()%500;
            mic_emit.tick_delay = 300+bt_get_rand()%300;
            mic_emit.init_state = MIC_EMIT_STA_DELAY;
        }
        break;

    case MIC_EMIT_STA_DELAY:
        if(tick_check_expire(mic_emit.ticks, mic_emit.tick_delay)) {
            mic_emit.init_state = MIC_EMIT_STA_START_ACTION;
        }
        break;
    }
}

AT(.text.func.process.mic_emit)
static void func_mic_emit_process(void)
{
#if TRACE_EN
    static u8 sta = 0xff;
    if(sta != mic_emit.init_state) {
        sta = mic_emit.init_state;
        TRACE("emit, state: %d\n", sta);
    }
#endif
    if(mic_emit.init_state != MIC_EMIT_STA_IDLE || wireless_mic.change_flag) {
        func_mic_emit_process_do();
    }

    wireless_mic_proc();
    func_process();

#if WARNING_MIC_MIX_WAV_PLAY_EN
    static u32 tick = 0;

    mic_mix_process();// 处理提示音混合
    if (tick_check_expire(tick,3000)) {
        tick = tick_get();
        if (wireless_mic_connected_sta_get())
        {
            msg_enqueue(EVT_TEST_WAV);
        }
    }
#endif // WARNING_MIC_MIX_WAV_PLAY_EN
    wireless_dump_proc();
}

AT(.text.func.mic_emit)
static void func_mic_emit_enter(void)
{
    lowpwr_pwroff_delay_reset();
    lowpwr_pwroff_auto_en();
    msg_queue_clear();

    func_mic_emit_init();
    bsp_ble_init();

#if GUI_LEDSEG_3P7S_EN
    ledseg_display(1);
#endif // GUI_LEDSEG_3P7S_EN
}

AT(.text.func.mic_emit)
static void func_mic_emit_exit(void)
{
    if (wireless_mic_connected_sta_get()) {

        wireless_tx_user_discon_cmd(); //通知接收端断开链接

        do {
            func_process();
        } while(wireless_mic_connected_sta_get());  //等待链接完全断开
    }

    if(sys_cb.mic_alg_en) {
        sys_cb.mic_alg_en = 0;                  //先关闭算法
        mic_emit_reset();
        sys_clk_free(INDEX_DECODE);             //再还原主频
    }

    bt_off();
    func_cb.last = FUNC_MIC_EMIT;
}

AT(.text.func.mic_emit)
void func_mic_emit(void)
{
    printf("%s\n", __func__);
    func_mic_emit_enter();

    while (func_cb.sta == FUNC_MIC_EMIT) {
        func_mic_emit_process();
        u16 msg = msg_dequeue();
        if (msg != MSG_NO) {
            func_mic_emit_message(msg);
        }
        //func_mic_emit_display();
    }

    func_mic_emit_exit();
}

