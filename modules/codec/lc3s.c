#include "include.h"


uint8_t cfg_lc3s_frame_len          = WIRELESS_MIC_FRAME_SIZE;
#if WIRELESS_MIC_FRAME_SIZE == 25
uint32_t cfg_lc3s_bitrate           = 80000;
#elif WIRELESS_MIC_FRAME_SIZE == 30
uint32_t cfg_lc3s_bitrate           = 96000;
#endif

