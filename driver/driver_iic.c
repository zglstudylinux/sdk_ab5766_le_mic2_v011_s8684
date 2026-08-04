/*
 * @File name    : driver_iic.c
 * @Author       : Bluetrum IOT Team
 * @Date         : 2025-09-02
 * @Description  : This file provides functions to manage the most functionalities
 *                 of the IIC peripheral.
 *
 * Copyright (c) by Bluetrum, All Rights Reserved.
 */
#include "driver_iic.h"


void iic_master_init(iic_typedef *iic, iic_master_init_typedef *iic_init_struct)
{
    uint32_t tmp_reg = iic->con0;

    tmp_reg &= ~IICCON0_POSDIV_MASK;
    tmp_reg |= ((iic_init_struct->scl_pose_div) << IICCON0_POSDIV_LSB) & IICCON0_POSDIV_MASK;

    tmp_reg &= ~IICCON0_HOLDCNT_MASK;
    tmp_reg |= ((iic_init_struct->sda_hold_cnt) << IICCON0_HOLDCNT_LSB) & IICCON0_HOLDCNT_MASK;

    tmp_reg &= ~IICCON0_MODE_BIT;
    tmp_reg |= (IIC_ROLE_MODE_MASTER << IICCON0_MODE_POS) & IICCON0_MODE_BIT;

    iic->con0 = tmp_reg;

    /*--- Enable model function ---*/
    RSTCON0 |= (1 << 3);
}

void iic_slave_init(iic_typedef *iic, iic_slave_init_typedef *iic_init_struct)
{
    uint32_t tmp_reg = iic->con0;

    tmp_reg &= ~IICCON0_MODE_BIT;
    tmp_reg |= (IIC_ROLE_MODE_SLAVE << IICCON0_MODE_POS) & IICCON0_MODE_BIT;

    iic->con0 = tmp_reg;

    iic->cmd_addr =  iic_init_struct->dev_addr & 0xfe;

    /*--- Enable model function ---*/
    RSTCON0 |= (1 << 3);
}


void iic_deinit(iic_typedef *iic)
{
    iic->con0 = 0;
    clk_gate2_cmd(CLK_GATE2_IIC, CLK_DIS);
}


void iic_cmd(iic_typedef *iic, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        iic->con0 |= IICCON0_IICEN;
    } else {
        iic->con0 &= ~IICCON0_IICEN;
    }
}

/**
  * @brief  Initializes the IIC control regisger according to param.
  * @param  transmit_mode: select the iic transmit direction.
  *         This parameter can be: IIC_SEND_MODE or IIC_RECEIVE_MODE.
  * @param  dev_addr: the slave device address.
  * @param  reg_addr: the address of the slave device register.
  * @param  len: the numbers of bytes of data.
  * @retval None
  */
static void iic_transmit_config(iic_typedef *iic, IIC_TRANSMIT_MODE transmit_mode, uint8_t dev_addr, uint16_t reg_addr, uint8_t len)
{
    iic->con1 = 0;
    iic->con1 |= IICCON1_STOP_EN;

    iic->con1 |= IICCON1_ADR0_EN;
    iic->con1 |= IICCON1_CTL0_EN;
    iic->con1 |= IICCON1_START0_EN;
    iic->con1 |= len & IICCON1_DATA_CNT;

    if (reg_addr & 0xff00) {
        iic->con1 |= IICCON1_ADR1_EN;
    }

    iic->cmd_addr = (dev_addr & 0xfe) | (reg_addr << 8);

    if (transmit_mode == IIC_SEND_MODE) {
        iic->con1 |= IICCON1_WDAT_EN;
    } else if (transmit_mode == IIC_RECEIVE_MODE) {
        iic->con1 |= IICCON1_TXNAK_EN;
        iic->con1 |= IICCON1_RDAT_EN;
        iic->con1 |= IICCON1_CTL1_EN;
        iic->con1 |= IICCON1_START1_EN;
        iic->cmd_addr |= ((dev_addr | 0x01) << 24);
    }

    iic->dma_cnt &= ~IICDMACNT_DMA_EN;
}


STATUS_STATE iic_master_send_data(iic_typedef *iic, uint8_t dev_addr, uint16_t reg_addr, uint32_t data, uint8_t len, uint8_t timeout_ms)
{
    iic_transmit_config(iic, IIC_SEND_MODE, dev_addr, reg_addr, len);

    iic->data = data;

    iic->con0 |= IICCON0_KS;

    if (timeout_ms == 0) {
        while ((iic->con0 & IICCON0_DONE) == RESET);
        iic->con0 |= IICCON0_CLR_DONE;
        return SUCCESS;
    } else if (timeout_ms == 0xff) {
        return SUCCESS;
    } else {
        while (timeout_ms) {
            if ((iic->con0 & IICCON0_DONE) != RESET) {
                iic->con0 |= IICCON0_CLR_DONE;
                return SUCCESS;
            }
            timeout_ms--;
            delay_ms(1);
        }
    }

    return FAILED;
}

AT(.com_periph.iic.dma)
STATUS_STATE iic_master_dma_start(iic_typedef *iic, IIC_TRANSMIT_MODE transmit_mode, uint8_t dev_addr, uint16_t reg_addr, uint8_t *buf, uint16_t len, uint8_t timeout_ms)
{
    iic_transmit_config(iic, transmit_mode, dev_addr, reg_addr, len);

    /* DMA Config */
    iic->dma_addr = ALIGN4_HI((u32)buf);
    iic->dma_cnt  = ((len - 1) << IICDMACNT_DMA_CNT_LSB);
    iic->dma_cnt |= IICDMACNT_DMA_EN;

    if (transmit_mode == IIC_SEND_MODE) {
        iic->dma_cnt |= IICDMACNT_DMA_ED_EN;        //DMA TX
    } else if (transmit_mode == IIC_RECEIVE_MODE) {
        iic->dma_cnt &= ~IICDMACNT_DMA_ED_EN;       //DMA RX
    }

    iic->con0 |= IICCON0_KS;

    if (timeout_ms == 0) {
        while ((iic->con0 & IICCON0_DONE) == RESET);
        iic->con0 |= IICCON0_CLR_DONE;
        return SUCCESS;
    } else if (timeout_ms == 0xff) {
        return SUCCESS;
    } else {
        while (timeout_ms) {
            if ((iic->con0 & IICCON0_DONE) != RESET) {
                iic->con0 |= IICCON0_CLR_DONE;
                return SUCCESS;
            }
            timeout_ms--;
            delay_ms(1);
        }
    }

    return SUCCESS;
}


STATUS_STATE iic_master_receive_data(iic_typedef *iic, uint8_t dev_addr, uint16_t reg_addr, uint32_t *data, uint8_t len, uint8_t timeout_ms)
{
    iic_transmit_config(iic, IIC_RECEIVE_MODE, dev_addr, reg_addr, len);

    iic->con0 |= IICCON0_KS;

    if (timeout_ms == 0) {
        while ((iic->con0 & IICCON0_DONE) == RESET);
        iic->con0 |= IICCON0_CLR_DONE;
        *data = iic->data;
        return SUCCESS;
    } else {
        while (timeout_ms) {
            if ((iic->con0 & IICCON0_DONE) != RESET) {
                iic->con0 |= IICCON0_CLR_DONE;
                *data = iic->data;
                return SUCCESS;
            }
            timeout_ms--;
            delay_ms(1);
        }
    }

    return FAILED;
}


void iic_pic_config(iic_typedef *iic, isr_t isr, int pr, IIC_IT_TYPEDEF interrupt_type, FUNCTIONAL_STATE state)
{
    uint32_t interrupt_bit = 0;
    uint32_t all_interrupt_type_mask = IIC_IT_DONE;

    if (interrupt_type == 0) {
        return;
    }

    if (interrupt_type & IIC_IT_DONE) {
        interrupt_bit |= IICCON0_INTEN;
    }

    if (state != DISABLE) {
        iic->con0 |= interrupt_bit;
        sys_irq_init(IRQn_IIC, pr, isr);
    } else {
        iic->con0 &= ~(uint32_t)interrupt_bit;
        if (interrupt_type == all_interrupt_type_mask) {
            PICEN &= ~BIT(IRQn_IIC);
        }
    }
}


AT(.com_periph.iic.get)
FLAG_STATE iic_get_flag(iic_typedef *iic, IIC_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_bit = 0;

    if (interrupt_type & IIC_IT_DONE) {
        interrupt_bit |= IICCON0_DONE;
    }

    if (interrupt_type & IIC_IT_ACK) {
        interrupt_bit |= IICCON0_ACKSTATUS;
    }

    if ((iic->con0 & interrupt_bit) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.iic.clear_flag)
void iic_clear_flag(iic_typedef *iic, IIC_IT_TYPEDEF interrupt_type)
{
    uint32_t interrupt_con_bit = 0;

    if (interrupt_type & IIC_IT_DONE) {
        interrupt_con_bit |= IICCON0_CLR_DONE;
    }

    iic->con0 |= interrupt_con_bit;
}


AT(.com_periph.iic.clear)
void iic_clear_all_flag(iic_typedef *iic)
{
    iic->con0 |= IICCON0_CLR_ALL;
}


AT(.com_periph.iic.nack_en)
void iic_slave_send_nack_en(iic_typedef *iic, FUNCTIONAL_STATE state)
{
    if (state != DISABLE) {
        iic->con0 |= IICCON0_RX_NACK_EN;
    } else {
        iic->con0 &= ~IICCON0_RX_NACK_EN;
    }
}


AT(.com_periph.iic.smp_sel)
void iic_slave_smp_sel(iic_typedef *iic, IIC_SMP_SEL_TYPEDEF edge)
{
    if (edge != IIC_SMP_SEL_FALLING) {
        iic->con0 |= IICCON0_SMP_SEL;
    } else {
        iic->con0 &= ~IICCON0_SMP_SEL;
    }
}

AT(.com_periph.iic.data_cnt_get)
uint8_t iic_slave_get_data_counter(iic_typedef *iic)
{
    return (iic->slave_sta & IICSSTS_DATA_CNT_MASK) >> IICSSTS_DATA_CNT_LSB;
}

AT(.com_periph.iic.data_send)
STATUS_STATE iic_slave_send_data(iic_typedef *iic, uint32_t data, uint8_t len)
{
    if (len > 4) {
        return FAILED;
    }

    iic->data = data;

    iic->con1 = len & IICCON1_DATA_CNT;

    //clr sta and kick
    iic->slave_sta |= IICSSTS_START_CLR | IICSSTS_RESTART_CLR | IICSSTS_STOP_CLR | IICSSTS_TXVLD_SET;

    return SUCCESS;
}


AT(.com_periph.iic.slave_data_send)
STATUS_STATE iic_slave_receive_data(iic_typedef *iic, uint8_t *data, uint8_t len)
{
    if (len > 4) {
        return FAILED;
    }

    uint32_t recv_data = iic->data;

    memcpy(data, &recv_data, len);

    iic->slave_sta |= IICSSTS_START_CLR | IICSSTS_RESTART_CLR | IICSSTS_STOP_CLR | IICSSTS_RXVLD_SET;

    return SUCCESS;
}


AT(.com_periph.iic.slave_sta)
FLAG_STATE iic_slave_get_sta(iic_typedef *iic, IIC_SLAVE_STA_TYPEDEF slave_sta)
{
    if ((iic->slave_sta & slave_sta) != RESET) {
        return SET;
    } else {
        return RESET;
    }
}

AT(.com_periph.iic.slave_sta)
FLAG_STATE iic_slave_clear_all_sta(iic_typedef *iic)
{
    iic->slave_sta |= IICSSTS_START_CLR | IICSSTS_RESTART_CLR | IICSSTS_STOP_CLR | IICSSTS_RXVLD_SET | IICSSTS_TXVLD_SET;

    return RESET;
}
