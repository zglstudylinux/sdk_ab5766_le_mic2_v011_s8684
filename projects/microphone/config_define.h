/*****************************************************************************
 * Module    : Configs
 * File      : config_define.h
 * Function  : 定义用户参数常量
 *****************************************************************************/
#ifndef CONFIG_DEFINE_H
#define CONFIG_DEFINE_H

/*****************************************************************************
 * Module    : FLASH大小定义
 *****************************************************************************/
#define FSIZE_1M                        0x100000
#define FSIZE_2M                        0x200000
#define FSIZE_512K                      0x80000
#define FSIZE_256K                      0x40000
#define FSIZE_128K                      0x20000

/*****************************************************************************
 * Module    : FOTA升级方式选择列表
 *****************************************************************************/
#define AB_FOT_TYPE_PACK                1   //FOTA压缩升级（代码做压缩处理，升级完成需做解压才可正常运行）
#define AB_FOT_TYPE_NORMAL              2   //FOTA非压缩升级（代码完全双备份，不做压缩处理，升级完成可直接运行）


/*****************************************************************************
 * Module    : 软关机模式定义
 *****************************************************************************/
#define PWROFF_NO_USED                  0
#define PWROFF_MODE1                    1
#define PWROFF_MODE2                    2

/*****************************************************************************
 * Module    : MIC模式定义
 *****************************************************************************/
#define MIC_MODE_NOT_RC                 0  //省RC
#define MIC_MODE_SINGLE                 1  //单端
#define MIC_MODE_DIFF                   2  //差分

/*****************************************************************************
* Module    : 采样率列表
*****************************************************************************/
#define SAMPLE_RATE_48K     0
#define SAMPLE_RATE_32K     1
#define SAMPLE_RATE_24K     2
#define SAMPLE_RATE_16K     3
#define SAMPLE_RATE_12K     4
#define SAMPLE_RATE_8K      5

/*****************************************************************************
* Module    : 编解码选择
*****************************************************************************/
#define CODEC_MONO              0           //flag
#define CODEC_STEREO            BIT(7)      //flag
#define CODEC_ADPCM             0           //index
#define CODEC_OPUS              1           //index
#define CODEC_MSBC              2           //index
#define CODEC_LC3               3           //index
#define CODEC_LC3S              4           //index
#define CODEC_SBT54B            5           //index
#define CODEC_SBT60B            6           //index
#define CODEC_SBC               7           //index
#define CODEC_ESBC              8           //index

/*****************************************************************************
 * Module    : GPIO 定义
 *****************************************************************************/
/* IO INDEX */
#define GPIO_NULL                       0x00
#define GPIO_PA0                        0xA0
#define GPIO_PA1                        0xA1
#define GPIO_PA2                        0xA2
#define GPIO_PA3                        0xA3
#define GPIO_PA4                        0xA4
#define GPIO_PA5                        0xA5
#define GPIO_PA6                        0xA6
#define GPIO_PA7                        0xA7
#define GPIO_PA8                        0xA8
#define GPIO_PA9                        0xA9
#define GPIO_PA10                       0xAA
#define GPIO_PA11                       0xAB
#define GPIO_PA12                       0xAC
#define GPIO_PA13                       0xAD
#define GPIO_PA14                       0xAE
#define GPIO_PA15                       0xAF
#define GPIO_PB0                        0xB0
#define GPIO_PB1                        0xB1
#define GPIO_PB2                        0xB2
#define GPIO_PB3                        0xB3
#define GPIO_PB4                        0xB4
#define GPIO_PB5                        0xB5
#define GPIO_PB6                        0xB6
#define GPIO_PB7                        0xB7
#define GPIO_PB8                        0xB8
#define GPIO_PB9                        0xB9
/* IO SWAPPER */
#define GPIO_PORT_GET(x)                (((x & 0xF0) == 0xA0)? GPIOA_REG: GPIOB_REG)
#define GPIO_PIN_GET(x)                 (1 << (x & 0x0F))

#endif //CONFIG_DEFINE_H
