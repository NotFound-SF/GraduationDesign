

#ifndef   __BSP_IIC_H
#define   __BSP_IIC_H


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f4xx.h"


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define         BSP_I2C_NBR_MAX                         3
#define         BSP_I2C_LOCK_TIME                       0

#define         BSP_I2C_NVIC_SUB_PRIO                   1
#define         BSP_I2C_NVIC_PRE_PRIO                   1

#define         BSP_I2C_ID_1                            0
#define         BSP_I2C_ID_2                            1
#define         BSP_I2C_ID_3                            2

#define        I2C_OWN_ADDRESS7                         0x01
#define        BSP_I2C_MODE_STANDARD_MAX_FREQ_HZ        100000
#define        BSP_I2C_MODE_FAST_MAX_FREQ_HZ            400000


#define        BSP_I2C_REG_SR1_BERR                     DEF_BIT_08
#define        BSP_I2C_REG_SR1_ARLO                     DEF_BIT_09
#define        BSP_I2C_REG_SR1_AF                       DEF_BIT_10
#define        BSP_I2C_REG_SR1_OVR                      DEF_BIT_11
#define        BSP_I2C_REG_SR1_PECERR                   DEF_BIT_12
#define        BSP_I2C_REG_SR1_TIMEOUT                  DEF_BIT_14
#define        BSP_I2C_REG_SR1_SMBALERT                 DEF_BIT_15


typedef enum {                                                      /*----------------- I2C工作模式 -------------*/
	BSP_I2C_MODE_STANDARD  = 0x00,                                  //标准模式低于100KHz
	BSP_I2C_MODE_FAST_2    = 0x01,                                  //快速模式低于400KHz 时钟低电平比高电平时间为2
	BSP_I2C_MODE_FAST_16_9 = 0x02                                   //快速模式低于400KHz 时钟低电平比高电平时间为16/9
}I2C_Mode_TypeDef;
	

typedef enum {                                                       /*--------------  I2C设备当前状态 ----------*/
	BSP_I2C_STATE_IDLE  = 0x00,                                      //空闲态                                         
	BSP_I2C_STATE_START = 0x01,                                      //起始位发送状态                                  
	BSP_I2C_STATE_ADDR  = 0x02,                                      //地址发送状态                                     
	BSP_I2C_STATE_DATA  = 0x03,                                      //数据发送状态                                   
	BSP_I2C_STATE_STOP  = 0x04                                       //停止位状态                                     
}I2C_State_TypeDef;


typedef enum {                                                       /*--------------  I2C访问类型  ------------*/
	BSP_I2C_ACCESS_TYPE_NONE  = 0x00,                                //未访问                                                          
	BSP_I2C_ACCESS_TYPE_RD    = 0x01,                                //读模式访问                                                      
	BSP_I2C_ACCESS_TYPE_WR    = 0x02,                                //写访问模式                                              
	BSP_I2C_ACCESS_TYPE_WR_RD = 0x03                                 //先写一个字节再连续读模式                               
}I2C_AccessType_TypeDef;


/*
*********************************************************************************************************
*                                               I2C1
*********************************************************************************************************
*/

#define         I2C1_GPIO_PORT_SCL                      GPIOB
#define         I2C1_SCL_PIN                            GPIO_Pin_8
#define         I2C1_SCL_PIN_SOURCE                     GPIO_PinSource8
#define         I2C1_GPIO_SCL_RCC                       RCC_AHB1Periph_GPIOB

#define         I2C1_GPIO_PORT_SDA                      GPIOB
#define         I2C1_SDA_PIN                            GPIO_Pin_9
#define         I2C1_SDA_PIN_SOURCE                     GPIO_PinSource9
#define         I2C1_GPIO_SDA_RCC                       RCC_AHB1Periph_GPIOB

#define         I2C1_PERIPH_RCC                         RCC_APB1Periph_I2C1
#define         I2C1_PERIPH_RCC_CMD                     RCC_APB1PeriphClockCmd
#define         BSP_I2C1_EV_IRQ                         I2C1_EV_IRQn
#define         BSP_I2C1_EV_Handler                     I2C1_EV_IRQHandler
#define         BSP_I2C1_ER_IRQ                         I2C1_ER_IRQn
#define         BSP_I2C1_ER_Handler                     I2C1_ER_IRQHandler        


/*
*********************************************************************************************************
*                                               I2C2
*********************************************************************************************************
*/

#define         I2C2_GPIO_PORT_SCL                      GPIOB
#define         I2C2_SCL_PIN                            GPIO_Pin_10
#define         I2C2_SCL_PIN_SOURCE                     GPIO_PinSource10
#define         I2C2_GPIO_SCL_RCC                       RCC_AHB1Periph_GPIOB

#define         I2C2_GPIO_PORT_SDA                      GPIOB
#define         I2C2_SDA_PIN                            GPIO_Pin_11
#define         I2C2_SDA_PIN_SOURCE                     GPIO_PinSource11
#define         I2C2_GPIO_SDA_RCC                       RCC_AHB1Periph_GPIOB

#define         I2C2_PERIPH_RCC                         RCC_APB1Periph_I2C2
#define         I2C2_PERIPH_RCC_CMD                     RCC_APB1PeriphClockCmd
#define         BSP_I2C2_EV_IRQ                         I2C2_EV_IRQn
#define         BSP_I2C2_EV_Handler                     I2C2_EV_IRQHandler
#define         BSP_I2C2_ER_IRQ                         I2C2_ER_IRQn
#define         BSP_I2C2_ER_Handler                     I2C2_ER_IRQHandler        


/*
*********************************************************************************************************
*                                               I2C3
*********************************************************************************************************
*/

#define         I2C3_GPIO_PORT_SCL                      GPIOA
#define         I2C3_SCL_PIN                            GPIO_Pin_8
#define         I2C3_SCL_PIN_SOURCE                     GPIO_PinSource8
#define         I2C3_GPIO_SCL_RCC                       RCC_AHB1Periph_GPIOA

#define         I2C3_GPIO_PORT_SDA                      GPIOC
#define         I2C3_SDA_PIN                            GPIO_Pin_9
#define         I2C3_SDA_PIN_SOURCE                     GPIO_PinSource9
#define         I2C3_GPIO_SDA_RCC                       RCC_AHB1Periph_GPIOC

#define         I2C3_PERIPH_RCC                         RCC_APB1Periph_I2C3
#define         I2C3_PERIPH_RCC_CMD                     RCC_APB1PeriphClockCmd
#define         BSP_I2C3_EV_IRQ                         I2C3_EV_IRQn
#define         BSP_I2C3_EV_Handler                     I2C3_EV_IRQHandler
#define         BSP_I2C3_ER_IRQ                         I2C3_ER_IRQn
#define         BSP_I2C3_ER_Handler                     I2C3_ER_IRQHandler        


/*
*********************************************************************************************************
*                                               FUNCTION
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Init (CPU_INT08U         i2c_id,
                           I2C_Mode_TypeDef   i2c_mode,
                           CPU_INT32U         clk_freq);

CPU_BOOLEAN  BSP_I2C_Rd (CPU_INT08U   i2c_id,
                         CPU_INT08U   i2c_addr,
                         CPU_INT08U  *p_buf,
                         CPU_INT16U   nbr_bytes);

CPU_BOOLEAN  BSP_I2C_Wr (CPU_INT08U   i2c_id,
                         CPU_INT08U   i2c_addr,
                         CPU_INT08U  *p_buf,
                         CPU_INT16U   nbr_bytes);

CPU_BOOLEAN  BSP_I2C_WrRd (CPU_INT08U   i2c_id,
                           CPU_INT08U   i2c_addr,
                           CPU_INT08U  *p_buf,
                           CPU_INT16U   nbr_bytes);






#endif    /* __BSP_IIC_H */



