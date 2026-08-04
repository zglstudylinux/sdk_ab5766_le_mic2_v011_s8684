/**
******************************************************************************************************************************
*
*@file ir_example.h
*
*@brief Header file for IR example
*
*Copyright (c) 2023, BLUETRUM
******************************************************************************************************************************
**/
#ifndef _IR_EXAMPLE_H
#define _IR_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#define IRTX_EXAMPLE_NEC            0
#define IRTX_EXAMPLE_TG9012         1
#define IRTX_EXAMPLE_RC5            2
#define IRTX_EXAMPLE_RC6            3
#define IRTX_EXAMPLE_CAPTURE_LEARN  4
#define IRTX_EXAMPLE_D7C6           5

#define IRRX_IO_PORT        GPIOA_REG
#define IRRX_IO_PIN         GPIO_PIN_0

#define IRTX_IO_PORT        GPIOA_REG
#define IRTX_IO_PIN         GPIO_PIN_1

typedef enum
{
    D7C6_PREAMBLE,
    D7C6_DATA,
    D7C6_CUSTOM,
}d7c6_component_t;


// This value is consistent with timing_freq
#define IR_TIMING_FREQ          1000000

#define T_US                    (1000 * 1000 / IR_TIMING_FREQ)	//X us per clock tick
//#define T_WIDTH                 (6 * IR_TIMING_FREQ / 10 / 1000)  //600us
#define TICK_CNT				(600 / T_US)  //clock tick counter for 600us

void irrx_example_init(void);
void irtx_example_init(uint8_t irtx_example_mode);
void irtx_extension_d7c6_example_init(void);
void irrx_handler_example(void);
void irtx_learn_example(void);
void irtx_send_example(uint8_t irtx_example_mode);
void irtx_extension_d7c6_handler_example(void);

void ir_learn_timer_send_init(void);
void ir_learn_tx_handler_example(void);
#ifdef __cplusplus
}
#endif

#endif
