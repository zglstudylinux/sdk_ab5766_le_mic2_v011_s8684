/*
 * @File name    : driver_rtc_reg.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-08-27
 * @Description  : This file contains all the functions prototypes for the RTC Base library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_RTC_REG_H
#define _DRIVER_RTC_REG_H

/*---------------------------------------------------------------------------------*/
/*                                     RTC                                         */
/*---------------------------------------------------------------------------------*/
/************************ Bit definition for RTCCON register ***********************/
#define RTCCON_VRTCVSEL                     ((uint32_t)0x00000001)

#define RTCCON_BAUDSEL                      ((uint32_t)0x00000006)

#define RTCCON_RTC1SIE                      ((uint32_t)0x00000008)
#define RTCCON_ALMIE                        ((uint32_t)0x00000010)
#define RTCCON_WKUPRSTEN                    ((uint32_t)0x00000020)
#define RTCCON_VUSBRSTEN                    ((uint32_t)0x00000040)
#define RTCCON_RTC_WKSLPEN                  ((uint32_t)0x00000080)
#define RTCCON_ALM_WKEN                     ((uint32_t)0x00000100)
#define RTCCON_VUSBON_WKSLPEN               ((uint32_t)0x00000200)
#define RTCCON_INBOX_WKSLPEN                ((uint32_t)0x00000400)
#define RTCCON_VUSBONIE                     ((uint32_t)0x00000800)
#define RTCCON_INBOXIE                      ((uint32_t)0x00001000)
#define RTCCON_RTC_ON                       ((uint32_t)0x00010000)
#define RTCCON_ALMPND                       ((uint32_t)0x00020000)
#define RTCCON_RTCWKSLPPND                  ((uint32_t)0x00040000)
#define RTCCON_RTCWKP                       ((uint32_t)0x00080000)
#define RTCCON_VUSBONLINE                   ((uint32_t)0x00100000)
#define RTCCON_VUSBOFF                      ((uint32_t)0x00200000)
#define RTCCON_INBOX                        ((uint32_t)0x00400000)

/************************ Bit definition for RTCCPND register **********************/
#define RTCCPND_TKTSOE                      ((uint32_t)0x00000003)

#define RTCCPND_CALMPND                     ((uint32_t)0x00020000)
#define RTCCPND_CWKSLPPND                   ((uint32_t)0x00040000)

#define RTCCPND_VBCNTDIS                    ((uint32_t)0x30000000)

#define RTCCPND_VBCNTKST                    ((uint32_t)0x40000000)
#define RTCCPND_VBCNTCLR                    ((uint32_t)0x80000000)

/************************ Bit definition for RTCCON11 register *********************/
#define RTCCON11_RTCWKSLPEN                 ((uint32_t)0x00000400)

#endif
