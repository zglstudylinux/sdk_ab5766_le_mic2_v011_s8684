#include "config.h"

ENTRY(_start)

//Define the flash max size
__max_flash_size = FLASH_CODE_SIZE;

__base = 0x10000000;

// Define the area size
__cache_stack_vma   = 0x10600;  //for loader
__cache_stack_size  = 512;      //for loader
__stack_vma         = 0x10800;
__stack_ram_size    = 1k;
__comm_ram_vma      = 0x10c00;
#if WIRELESS_CON_CODEC_SEL == CODEC_ESBC
__comm_ram_size     = 30k;
#else
__comm_ram_size     = 27k;
#endif

__adapter_ram_vma   = __comm_ram_vma + __comm_ram_size;
__adapter_ram_size  = (61k - __comm_ram_size);

__emit_ram_vma      = __adapter_ram_vma;
__emit_ram_size     = __adapter_ram_size;

__test_ram_vma      = __adapter_ram_vma;
__test_ram_size     = __adapter_ram_size;
/*
    0x18000-0x1e000用来复用处理echo/reverb
    当WIRELESS_MIC_ECHO_EN或者WIRELESS_MIC_ROOM_REVERB_EN打开，就会用这一部分存放算法使用的大部分buf
    当WIRELESS_MIC_ECHO_EN和WIRELESS_MIC_ROOM_REVERB_EN均关闭，释放该部分
*/
__mram_vma          = 0x18000;
__mram_ram_size     = 32k;

MEMORY
{
    init            : org = __base,             len = 512
    flash(rx)       : org = __base + 512,       len = __max_flash_size
    comm(rx)        : org = __comm_ram_vma,     len = __comm_ram_size

    comm_adapter    : org = __adapter_ram_vma,  len = __adapter_ram_size
    comm_emit       : org = __emit_ram_vma,     len = __emit_ram_size
    comm_test       : org = __test_ram_vma,     len = __test_ram_size

    stack           : org = __stack_vma,        len = __stack_ram_size
    mram            : org = __mram_vma,         len = __mram_ram_size

}

SECTIONS
{
    .init : {
        *(.reset)
    } > init

    .code_comm : {
        *(.vector)
        *(.com_text*)
        *(.bcom_text*)
        *(.bcom_rodata*)
        *(.com_text.stack.handler)

        *(.rodata.adpcm_com*)
        *(.text.sbc_com*)
#if WIRELESS_MIC_KEY_PRESS_MUTE_EN
        *(.text.bsp_key_scan*)
        *(.rodata.bsp_key_scan*)
#endif
        *(.rodata.sbc_com*)
        *(.com_rodata*)
        *debug.o(.rodata*)

        __code_start_escb_com = .;
        *(.rodata.esbc_com*)
        *(.text.esbc_com*)
        __code_end_escb_com = .;

        *(.buf.esbc_com)
        *(.com_periph.*)
        *(.data*)
        *(.sdata*)
        *(.mp3_text*)

#if ADAPTER_USB_MIC_RX_EN || WIRELESS_MIC_32K_EN
        __code_start_lib_src = .;
        *(.text.src_proc*)
        *(.rodata.src*)
        __code_end_lib_src = .;
#endif // ADAPTER_USB_MIC_RX_EN

        . = ALIGN(4);
    } > comm AT > flash

    .code_adapter : {
        __code_start_usbdev = .;
        *(.usbdev.com*)
        __code_end_usbdev = .;

        __code_start_plc_soft = .;
        *(.text.plc_proc*)
        *(.rodata.plc*)
        __code_end_plc_soft = .;

        __code_start_sbc_enc = .;
        *(.rodata.sbc_dec*)
        *(.text.sbc_dec*)
        __code_end_sbc_enc = .;

        __code_start_adpcm_dec = .;
        *(.text.adpcm_dec*)
        __code_end_adpcm_dec = .;

        __code_start_escb_dec = .;
        *(.text.esbc_dec*)
        *(.rodata.esbc_dec*)
        __code_end_escb_dec = .;

        __code_start_lc3_dec = .;
        *(.text.lc3_dec*)
        *(.rodata.lc3_dec*)
        . = ALIGN(4);
        __code_end_lc3_dec = .;

        __code_start_adapter_proc = .;
        *(.text.adapter.proc*)
        __code_end_adapter_proc = .;

#if ADAPTER_MIX_DRC_EN
         __code_start_mic_mix = .;
        *(.text.mic_mix_proc*)
         __code_end_mic_mix = .;
#endif
        . = ALIGN(4);
    } > comm_adapter AT > flash

    .data_adapter (NOLOAD) : {
        *(.buf.adapter.proc*)
        *(.buf.iis_buf*)
        __buf_start_usbdev = .;
        *(.buf.usb*)
        *(.usb_buf*)
        *(.usb*)
        *(.ude.aubuf*)
        __buf_end_usbdev = .;

        __buf_start_sbc_dec = .;
        *(.buf.sbc_dec*)
        __buf_end_sbc_dec = .;

        __buf_start_plc_soft = .;
        *(.buf.plc_buf*)
        __buf_end_plc_soft = .;

        __buf_start_esbc_dec = .;
        *(.buf.esbc_dec)
        __buf_end_esbc_dec = .;

        __buf_start_lc3dec = .;
        *(.buf.lc3_dec*)
        __buf_end_lc3dec = .;

#if ADAPTER_MIX_DRC_EN
         __buf_start_mic_mix = .;
        *(.buf.mic_mix*)
         __buf_end_mic_mix = .;
#endif

        *(.buf.sddac*)
    } > comm_adapter


    .code_emit : {
        __code_start_sbc_enc = .;
        *(.rodata.sbc_enc*)
        *(.text.sbc_enc*)
        __code_end_sbc_enc = .;

        __code_start_lc3com = .;
        *(.text.lc3_com*)
        *(.rodata.lc3)
        __code_end_lc3com = .;

        __code_start_lc3enc = .;
        *(.text.lc3_enc*)
        *(.rodata.lc3_enc*)
        __code_end_lc3enc = .;

#if WIRELESS_MIC_AINS4_32K_EN
        __code_start_ains4_32k = .;
        *(.text.ains4_32k_proc*)
        *(.rodata.ains4_32k*)
        __code_end_ains4_32k = .;
#endif

#if WIRELESS_MIC_MAGIC_EN
        __code_start_pitch = .;
        *(.text.pitch_shift2_proc*)
        *(.rodata.pitch_shift2*)
        __code_end_pitch = .;
#endif

        __code_start_mic_emit_proc = .;
        *(.text.mic_emit.proc*)
        *(.rodata.mic_emit.proc*)
        __code_end_mic_emit_proc = .;

        __code_start_freq_shift2 = .;
        *(.text.freq_shift2_proc*)
        *(.rodata.freq_shift2*)
        __code_end_freq_shift2 = .;

#if WIRELESS_MIC_DNR_FRE_EN
        __code_start_dnr_fre = .;
        *(.text.dnr_fre_proc*)
        *(.rodata.dnr_fre*)
        __code_end_dnr_fre = .;
#endif

#if WIRELESS_ALLPASS_FILTER_CHANGE_EN
        __code_start_allpass_filter_change = .;
        *(.text.allpass_filter_change_proc*)
        __code_end_allpass_filter_change = .;
#endif

#if WIRELESS_MIC_ECHO_EN
        __code_start_echo = .;
        *(.text.echo_proc*)
        *(.rodata.echo*)
        __code_end_echo = .;
#endif

#if WIRELESS_MIC_ROOM_REVERB_EN
        __code_start_room_reverb = .;
        *(.text.room_reverb_proc*)
        *(.rodata.room_reverb*)
        __code_end_room_reverb = .;
#endif

#if WIRELESS_MIC_ROBOTIZATION_EN
        __code_start_robotization = .;
        *(.text.robotization_proc*)
        *(.rodata.robotization*)
        __code_end_robotization = .;
#endif

#if WIRELESS_MIC_AGC_EN
        __code_start_agc = .;
        *(.text.agc_proc*)
        *(.rodata.agc*)
        __code_end_agc = .;
#endif

        __code_start_adpcm_enc = .;
        *(.text.adpcm_enc*)
        __code_enc_adpcm_enc = .;

        __code_start_escb_enc = .;
        *(.text.esbc_enc*)
        *(.rodata.esbc_enc*)
        __code_end_escb_enc = .;

        *(.text.mic_emit.proc*)
        . = ALIGN(4);
    } > comm_emit AT > flash

    .data_emit (NOLOAD) : {
        __buf_start_sbc_enc = .;
        *(.buf.sbc_enc*)
        __buf_end_sbc_enc = .;

        __buf_start_lc3enc = .;
        *(.buf.lc3_enc*)
        __buf_end_lc3enc = .;

#if WIRELESS_MIC_AINS4_32K_EN
        __buf_start_ains4_32k = .;
        *(.buf.ains4_32k*)
        __buf_end_ains4_32k = .;
#endif

#if WIRELESS_MIC_DNR_FRE_EN
        __buf_start_dnr_fre = .;
        *(.buf.dnr_fre*)
        __buf_end_dnr_fre = .;
#endif

#if WIRELESS_MIC_EQ_DRC_EN
         __buf_start_mic_effect = .;
        *(.buf.mic_effect*)
         __buf_end_mic_effect = .;
#endif

#if WIRELESS_MIC_MAGIC_EN
        __buf_start_pitch = .;
        *(.buf.pitch_shift*)
        __buf_end_pitch = .;
#endif

#if WIRELESS_ALLPASS_FILTER_CHANGE_EN
         __buf_start_allpass_filter_change = .;
        *(.buf.allpass_filter_change*)
         __buf_end_allpass_filter_change = .;
#endif

#if WIRELESS_MIC_ROBOTIZATION_EN
        __buf_start_robotization = .;
        *(.buf.robotization*)
        __buf_end_robotization = .;
#endif

        __buf_start_esbc_enc = .;
        *(.buf.esbc_enc)
        __buf_end_esbc_enc = .;

        __buf_start_freq_shift2 = .;
        *(.buf.freq_shift2*)
        __buf_end_freq_shift2 = .;

#if WIRELESS_MIC_ROOM_REVERB_EN
        __buf_start_room_reverb = .;
        *(.buf.room_reverb*)
        __buf_end_room_reverb = .;
#endif

#if WIRELESS_MIC_ECHO_EN
        __buf_start_echo = .;
        *(.buf.echo*)
        __buf_end_echo = .;
#endif

#if WIRELESS_MIC_AGC_EN
        __buf_start_agc = .;
        *(.buf.agc*)
        __buf_end_agc = .;
#endif

        *(.buf.mic_emit.proc*)
        *(.buf.sdadc*)
        *(.buf.online_effect*)
        . = ALIGN(4);
    } > comm_emit

    .mram_echo __mram_vma (NOLOAD) : {

        . = 0x6000;
    } > mram

    .mram_reverb __mram_vma (NOLOAD) : {

        . = 0x6000;
    } > mram

    .code_test : {
        __code_start_le_dut = .;
        *(.text.le_dut*)
        *(.rodata.le_dut*)
        __code_end_le_dut = .;
    } > comm_test AT > flash

    .data_test (NOLOAD): {
        __buf_start_le_dut = .;
        *(.buf.le_dut*)
        __buf_end_le_dut = .;
    } > comm_test

    .flash : {
        __code_start_ws_mic = .;
        *(.text.ws_mic_com*)
        . = ALIGN(512);
        __code_end_ws_mic = .;

        __code_start_sleep_send = .;
        *(.com_sleep*)
        . = ALIGN(512);
        __code_end_sleep_send = .;

        __code_start_charge = .;
        *(.charge_text*)
        *(.text.bsp.charge)
        *(.text.bsp.tkey)
        __code_end_charge = .;

        __code_start_pwrsave = .;
        *(.text.pwroff*)
        __code_end_pwrsave = .;

        __code_start_bb_test = .;
        *(.bb_test*)
        *(.bb_test_rodata*)
        __code_end_bb_test = .;

        __code_start_update = .;
        *(.text.update*)
        __code_end_update = .;

        __code_start_fota = .;
        *(.text.fot.cache*)
        __code_end_fota = .;

        *(.irq_init.aligned)
//        . = ALIGN(512);
        *(.text.ble.chmap*)
        *(.text*)
        *(.rodata*)
        *(.srodata*)
        *(.rela*)
        LONG(0)
        . = ALIGN(512);

    } > flash

    .stack (NOLOAD) : {
        __irq_stack_start = .;
        . = 0x400;
        __irq_stack = .;
    } > stack
    __irq_stack_size = __irq_stack - __irq_stack_start;

    .data_comm (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        *(.sbss*)
        *(.buf*)
        *(.sdadc_buf*)
        *(.sddac_buf*)
        __bss_end = .;

        __bram_start = .;
        *(.btmem.bthw)
        *(.btmem*)

        *(.ble_cache*)
        *(.ble_buf*)
        __bram_end = .;

#if ADAPTER_USB_MIC_RX_EN || WIRELESS_MIC_32K_EN
        __buf_start_lib_src = .;
        *(.buf.src*)
        __buf_end_lib_src = .;
#endif // ADAPTER_USB_MIC_RX_EN

        __heap_start = .;
        *(.mem_heap)
        __heap_end = .;
    } > comm
}

//Calc the lma
__bank_size = SIZEOF(.flash);

__sys_size = __stack_ram_size;
__bss_size = __bss_end - __bss_start;
__heap_size = __heap_end - __heap_start;
__comm_vma = __comm_ram_vma;
__comm_lma = LOADADDR(.code_comm);
__comm_size = SIZEOF(.code_comm);
__bram_size = __bram_end - __bram_start;
__total_size = __sys_size + __bss_size + __comm_size + __bram_size;

__comm_adapter_vma = __adapter_ram_vma;
__comm_apapter_lma = LOADADDR(.code_adapter);
__comm_apapter_size = SIZEOF(.code_adapter);
__comm_emit_vma = __emit_ram_vma;
__comm_emit_lma = LOADADDR(.code_emit);
__comm_emit_size = SIZEOF(.code_emit);
__comm_test_vma = __test_ram_vma;
__comm_test_lma = LOADADDR(.code_test);
__comm_test_size = SIZEOF(.code_test);

__usbdev_code_size  = __code_end_usbdev - __code_start_usbdev;
__usbdev_buf_size   = __buf_end_usbdev - __buf_start_usbdev;

