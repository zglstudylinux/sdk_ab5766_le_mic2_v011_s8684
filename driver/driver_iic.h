/*
 * @File name    : driver_iic.h
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-02
 * @Description  : This file contains all the functions prototypes for the IIC library.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#ifndef _DRIVER_IIC_H
#define _DRIVER_IIC_H

#include "driver_com.h"

/**
 * @brief IIC IT and Flag Definition
 */
typedef enum {
    IIC_IT_DONE             = 0x01,
    IIC_IT_ACK              = 0x02,
} IIC_IT_TYPEDEF;


/**
 * @brief IIC Role Mode Enumeration
 */
typedef enum {
    IIC_ROLE_MODE_MASTER                    = 0,
    IIC_ROLE_MODE_SLAVE                     = 1,
} IIC_ROLE_MODE_TYPEDEF;

/**
 * @brief IIC Data Sample Edge Selection In Slave Mode.
 */
typedef enum {
    IIC_SMP_SEL_FALLING                     = 0,
    IIC_SMP_SEL_RISING                      = 1,
} IIC_SMP_SEL_TYPEDEF;

/**
 * @brief IIC Transmit Direction Enumeration
 */
typedef enum {
    IIC_SEND_MODE           = 1,
    IIC_RECEIVE_MODE        = 2,
} IIC_TRANSMIT_MODE;

typedef enum {
    IIC_SLAVE_STA_RXVLD     = 1 << 0,
    IIC_SLAVE_STA_TXVLD     = 1 << 1,
    IIC_SLAVE_STA_START     = 1 << 2,
    IIC_SLAVE_STA_RESTART   = 1 << 3,
    IIC_SLAVE_STA_STOP      = 1 << 4,
    IIC_SLAVE_STA_SRX       = 1 << 5,
    IIC_SLAVE_STA_STX       = 1 << 6,
} IIC_SLAVE_STA_TYPEDEF;

/**
 * @brief IIC Master Init Structure
 */
typedef struct {
    /*--- Configure the iic SCL pose div conter ---*/
    uint8_t                 scl_pose_div;
    /*--- Configure the iic SDA hold cnt when SCL falling ---*/
    uint8_t                 sda_hold_cnt;
} iic_master_init_typedef;

/**
 * @brief IIC Slave Init Structure
 */
typedef struct {
    /*--- Configure the iic device address ---*/
    uint8_t                 dev_addr;
} iic_slave_init_typedef;

//--------------- Function used to configure iic or transmit data ---------------//
/**
  * @brief  Initializes the IIC peripheral according to the specified
  *         parameters in the iic_master_init_typedef.
  * @param  iic_init_struct: pointer to a iic_master_init_typedef structure that
  *         contains the configuration information for the specified IIC peripheral.
  * @retval None
  */
void iic_master_init(iic_typedef *iic, iic_master_init_typedef *iic_init_struct);

/**
  * @brief  Initializes the IIC peripheral according to the specified
  *         parameters in the iic_master_init_typedef.
  * @param  iic_init_struct: pointer to a iic_slave_init_typedef structure that
  *         contains the configuration information for the specified IIC peripheral.
  * @retval None
  */
void iic_slave_init(iic_typedef *iic, iic_slave_init_typedef *iic_init_struct);

/**
 * @brief  De-initialize the specified IIC peripheral.
 * @retval None
 */
void iic_deinit(iic_typedef *iic);

/**
  * @brief  Enable or disable the specified IIC peripheral.
  * @param  state: state of the IIC peripheral.
  *         This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void iic_cmd(iic_typedef *iic, FUNCTIONAL_STATE state);

/**
  * @brief  Send data by iic bus, this function will block program.
  * @param  dev_addr: the slave device address.
  * @param  reg_addr: the address of the slave device register.
  * @param  data: the data needed to transmit.
  * @param  len: the numbers of bytes of data. A maximum of 4 bytes can be
  *              transmitted at a time. So len should less than or equal to 4.
  * @param  timeout_ms: timeout period for waiting for a result, uint:1ms. 0:continuous waiting; 0xff:no waiting
  * @retval The status of result. It can be (FAILED or SUCCESS).
  */
STATUS_STATE iic_master_send_data(iic_typedef *iic, uint8_t dev_addr, uint16_t reg_addr, uint32_t data, uint8_t len, uint8_t timeout_ms);

STATUS_STATE iic_master_dma_start(iic_typedef *iic, IIC_TRANSMIT_MODE transmit_mode, uint8_t dev_addr, uint16_t reg_addr, uint8_t *buf, uint16_t len, uint8_t timeout_ms);

/**
  * @brief  Receive data by iic bus, this function will block program.
  * @param  dev_addr: the slave device address.
  * @param  reg_addr: the address of the slave device.
  * @param  len: the numbers of bytes of data.
  * @param  timeout_ms: timeout period for waiting for a result.
  * @retval The status of result. It can be (FAILED or SUCCESS).
  */
STATUS_STATE iic_master_receive_data(iic_typedef *iic, uint8_t dev_addr, uint16_t reg_addr, uint32_t *data, uint8_t len, uint8_t timeout_ms);
//------------------- Function used to configure interrupt -------------------//
/**
  * @brief  Enable or disable the specified IIC interrupt.
  * @param  isr: Function to be executed for service interruption.
  * @param  pr: Priority of service interruption.
  * @param  interrupt_type: specifies the IIC interrupt sources to be enable or disable.
  *         this parameter can be one of the following values:
  *             @arg IIC_IT_DONE: IIC transmit finished interrupt.
  * @param  state: the state of the IIC peripheral.
  *         this parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void iic_pic_config(iic_typedef *iic, isr_t isr, int pr, IIC_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state);

/**
  * @brief  Get the IIC interrupt pending.
  * @param  interrupt_type: specifies the flag to get.
  *         this parameter can be one of the following values:
  *             @arg IIC_IT_DONE: IIC transmit finished flag.
  *             @arg IIC_IT_ACK: IIC receive slave device ack flag.
  * @retval The state of iic_flag (SET or RESET).
  */
FLAG_STATE iic_get_flag(iic_typedef *iic, IIC_IT_TYPEDEF interrupt_type);

/**
  * @brief  Clear the IIC pending flag.
  * @param  iic: IIC_REG
  * @param  interrupt_type: specifies the flag to check.
  *         this parameter can be one of the following values:
  *             @arg IIC_IT_DONE: IIC transmit finished flag..
  * @retval None
  */
void iic_clear_flag(iic_typedef *iic, IIC_IT_TYPEDEF interrupt_type);

/**
  * @brief  Clear all status.
  * @retval None
  */
void iic_clear_all_flag(iic_typedef *iic);

/**
  * @brief  Send NACK to the master when in slave mode.
  * @retval None
  */
void iic_slave_send_nack_en(iic_typedef *iic, FUNCTIONAL_STATE state);

/**
  * @brief  Data sample edge selection in slave mode.
  * @retval None
  */
void iic_slave_smp_sel(iic_typedef *iic, IIC_SMP_SEL_TYPEDEF edge);

/**
  * @brief  Get RX/TX data counter.
  * @param  iic: IIC_REG
  * @retval Data counter
  */
uint8_t iic_slave_get_data_counter(iic_typedef *iic);

/**
  * @brief  Send data by iic bus, valid only in slave mode.
  * @param  iic: IIC_REG
  * @param  data: the data needed to transmit.
  * @param  len: the numbers of bytes of data. A maximum of 4 bytes can be
  *              transmitted at a time. So len should less than or equal to 4.
  * @retval The status of result. It can be (FAILED or SUCCESS).
  */
STATUS_STATE iic_slave_send_data(iic_typedef *iic, uint32_t data, uint8_t len);

/**
  * @brief  Receive data by iic bus, valid only in slave mode.
  * @param  iic: IIC_REG
  * @param  data: the data buf for receiving data.
  * @param  len: the numbers of bytes of data. A maximum of 4 bytes can be
  *              transmitted at a time. So len should less than or equal to 4.
  * @retval The status of result. It can be (FAILED or SUCCESS).
  */
STATUS_STATE iic_slave_receive_data(iic_typedef *iic, uint8_t *data, uint8_t len);

/**
  * @brief  Get the receiving status of the iic slave device.
  * @param  iic: IIC_REG
  * @param  slave_sta: the state that needs to be get.
  * @retval The status of result. It can be (FAILED or SUCCESS).
  */
FLAG_STATE iic_slave_get_sta(iic_typedef *iic, IIC_SLAVE_STA_TYPEDEF slave_sta);

/**
  * @brief  Clear all the receiving status of the iic slave device.
  * @param  iic: IIC_REG
  * @retval The status of result. It can be (FAILED or SUCCESS).
  */
FLAG_STATE iic_slave_clear_all_sta(iic_typedef *iic);

#endif // _DRIVER_IIC_H
