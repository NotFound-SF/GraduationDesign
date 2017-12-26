
#ifndef     __BSP_SRAM_H
#define     __BSP_SRAM_H

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

#define    BSP_SRAM_Bank                             FSMC_Bank1_NORSRAM3
#define    BSP_SRAM_BASE                             0x68000000
#define    BSP_SRAM_SIZE                             0x100000

/*
*********************************************************************************************************
*                                               SRAM控制引脚定义
*********************************************************************************************************
*/

/*-----------------------------  移植时需要修改该引脚定义  ------------------------*/
#define    SRAM_GPIO_PORT_NCE                     GPIOG
#define    SRAM_GPIO_PORT_RCC_NCE                 RCC_AHB1Periph_GPIOG
#define    SRAM_PIN_NCE                           GPIO_Pin_10  
#define    SRAM_PIN_SOURCE_NCE                    GPIO_PinSource10




#define    SRAM_GPIO_PORT_NLB                     GPIOE
#define    SRAM_GPIO_PORT_RCC_NLB                 RCC_AHB1Periph_GPIOE
#define    SRAM_PIN_NLB                           GPIO_Pin_0  
#define    SRAM_PIN_SOURCE_NLB                    GPIO_PinSource0

#define    SRAM_GPIO_PORT_NUB                     GPIOE
#define    SRAM_GPIO_PORT_RCC_NUB                 RCC_AHB1Periph_GPIOE
#define    SRAM_PIN_NUB                           GPIO_Pin_1  
#define    SRAM_PIN_SOURCE_NUB                    GPIO_PinSource1


/*
*********************************************************************************************************
*                                               FUNCTION
*********************************************************************************************************
*/

void       BSP_SRAM_Init      (void);

#endif



