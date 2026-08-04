#include "include.h"
#include "driver_lowpwr.h"


int main(void)
{
    sys_cb.rst_reason = sys_rst_init(WK0_10S_RESET);
    printf("Hello AB5766: %08x\n", sys_cb.rst_reason);
    printf("SDK: v%04X LIBS: v%04X\n", SDK_VERSION, LIBS_VERSION);
    sys_rst_dump(sys_cb.rst_reason);

    sys_cb.wakeup_reason  = lowpwr_get_wakeup_source();

    printf("SW_VERSION: %s\n",SW_VERSION);

    code_info_dump();

    bsp_sys_init();
    func_run();

    return 0;
}
