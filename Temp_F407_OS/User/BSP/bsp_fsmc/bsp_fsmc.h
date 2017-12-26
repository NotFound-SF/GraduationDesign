
#ifndef       __BSP_FSMC_H
#define       __BSP_FSMC_H


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

#define     FSMC_PERIPH_RCC_CMD                  RCC_AHB3PeriphClockCmd
#define    	FSMC_PERIPH_RCC                      RCC_AHB3Periph_FSMC


/*
*********************************************************************************************************
*                                               地址引脚定义
*********************************************************************************************************
*/

/*-----------------------------    A0-A9引脚宏定义  ---------------------------*/

#define     FSMC_GPIO_PORT_A0_9                   GPIOF
#define     FSMC_GPIO_PORT_RCC_A0_9               RCC_AHB1Periph_GPIOF

#define     FSMC_PIN_A0                           GPIO_Pin_0
#define     FSMC_PIN_SOURCE_A0                    GPIO_PinSource0
#define     FSMC_PIN_A1                           GPIO_Pin_1
#define     FSMC_PIN_SOURCE_A1                    GPIO_PinSource1
#define     FSMC_PIN_A2                           GPIO_Pin_2
#define     FSMC_PIN_SOURCE_A2                    GPIO_PinSource2
#define     FSMC_PIN_A3                           GPIO_Pin_3
#define     FSMC_PIN_SOURCE_A3                    GPIO_PinSource3
#define     FSMC_PIN_A4                           GPIO_Pin_4
#define     FSMC_PIN_SOURCE_A4                    GPIO_PinSource4
#define     FSMC_PIN_A5                           GPIO_Pin_5
#define     FSMC_PIN_SOURCE_A5                    GPIO_PinSource5
#define     FSMC_PIN_A6                           GPIO_Pin_12
#define     FSMC_PIN_SOURCE_A6                    GPIO_PinSource12
#define     FSMC_PIN_A7                           GPIO_Pin_13
#define     FSMC_PIN_SOURCE_A7                    GPIO_PinSource13
#define     FSMC_PIN_A8                           GPIO_Pin_14
#define     FSMC_PIN_SOURCE_A8                    GPIO_PinSource14
#define     FSMC_PIN_A9                           GPIO_Pin_15
#define     FSMC_PIN_SOURCE_A9                    GPIO_PinSource15

#define     FSMC_PINS_A0_9                        (FSMC_PIN_A0   | \
												   FSMC_PIN_A1   | \
												   FSMC_PIN_A2   | \
												   FSMC_PIN_A3   | \
												   FSMC_PIN_A4   | \
												   FSMC_PIN_A5   | \
												   FSMC_PIN_A6   | \
												   FSMC_PIN_A7   | \
												   FSMC_PIN_A8   | \
												   FSMC_PIN_A9)
												   

/*----------------------------    A10-A15引脚宏定义  --------------------------*/

#define     FSMC_GPIO_PORT_A10_15                  GPIOG
#define     FSMC_GPIO_PORT_RCC_A10_15              RCC_AHB1Periph_GPIOG

#define     FSMC_PIN_A10                           GPIO_Pin_0
#define     FSMC_PIN_SOURCE_A10                    GPIO_PinSource0
#define     FSMC_PIN_A11                           GPIO_Pin_1
#define     FSMC_PIN_SOURCE_A11                    GPIO_PinSource1
#define     FSMC_PIN_A12                           GPIO_Pin_2
#define     FSMC_PIN_SOURCE_A12                    GPIO_PinSource2
#define     FSMC_PIN_A13                           GPIO_Pin_3
#define     FSMC_PIN_SOURCE_A13                    GPIO_PinSource3
#define     FSMC_PIN_A14                           GPIO_Pin_4
#define     FSMC_PIN_SOURCE_A14                    GPIO_PinSource4
#define     FSMC_PIN_A15                           GPIO_Pin_5
#define     FSMC_PIN_SOURCE_A15                    GPIO_PinSource5

#define     FSMC_PINS_A10_15                      (FSMC_PIN_A10   | \
												   FSMC_PIN_A11   | \
												   FSMC_PIN_A12   | \
												   FSMC_PIN_A13   | \
												   FSMC_PIN_A14   | \
												   FSMC_PIN_A15)
					
					
/*----------------------------    A16-A18引脚宏定义  --------------------------*/

#define     FSMC_GPIO_PORT_A16_18                  GPIOD
#define     FSMC_GPIO_PORT_RCC_A16_18              RCC_AHB1Periph_GPIOD

#define     FSMC_PIN_A16                           GPIO_Pin_11
#define     FSMC_PIN_SOURCE_A16                    GPIO_PinSource11
#define     FSMC_PIN_A17                           GPIO_Pin_12
#define     FSMC_PIN_SOURCE_A17                    GPIO_PinSource12
#define     FSMC_PIN_A18                           GPIO_Pin_13
#define     FSMC_PIN_SOURCE_A18                    GPIO_PinSource13


#define     FSMC_PINS_A16_18                       (FSMC_PIN_A16   | \
												    FSMC_PIN_A17   | \
												    FSMC_PIN_A18)  


/*
*********************************************************************************************************
*                                               数据引脚定义
*********************************************************************************************************
*/

/*----------------------------    D0-D3 引脚宏定义   --------------------------*/
#define     FSMC_GPIO_PORT_D0_3                   GPIOD
#define     FSMC_GPIO_PORT_RCC_D0_3               RCC_AHB1Periph_GPIOD

#define     FSMC_PIN_D0                           GPIO_Pin_14
#define     FSMC_PIN_SOURCE_D0                    GPIO_PinSource14
#define     FSMC_PIN_D1                           GPIO_Pin_15
#define     FSMC_PIN_SOURCE_D1                    GPIO_PinSource15
#define     FSMC_PIN_D2                           GPIO_Pin_0
#define     FSMC_PIN_SOURCE_D2                    GPIO_PinSource0
#define     FSMC_PIN_D3                           GPIO_Pin_1
#define     FSMC_PIN_SOURCE_D3                    GPIO_PinSource1

#define     FSMC_PINS_D0_3                       (FSMC_PIN_D0   | \
												  FSMC_PIN_D1   | \
												  FSMC_PIN_D2   | \
												  FSMC_PIN_D3)

/*----------------------------    D4-D12引脚宏定义   --------------------------*/

#define     FSMC_GPIO_PORT_D4_12                  GPIOE
#define     FSMC_GPIO_PORT_RCC_D4_12              RCC_AHB1Periph_GPIOE

#define     FSMC_PIN_D4                           GPIO_Pin_7
#define     FSMC_PIN_SOURCE_D4                    GPIO_PinSource7
#define     FSMC_PIN_D5                           GPIO_Pin_8
#define     FSMC_PIN_SOURCE_D5                    GPIO_PinSource8
#define     FSMC_PIN_D6                           GPIO_Pin_9
#define     FSMC_PIN_SOURCE_D6                    GPIO_PinSource9
#define     FSMC_PIN_D7                           GPIO_Pin_10
#define     FSMC_PIN_SOURCE_D7                    GPIO_PinSource10
#define     FSMC_PIN_D8                           GPIO_Pin_11
#define     FSMC_PIN_SOURCE_D8                    GPIO_PinSource11
#define     FSMC_PIN_D9                           GPIO_Pin_12
#define     FSMC_PIN_SOURCE_D9                    GPIO_PinSource12
#define     FSMC_PIN_D10                          GPIO_Pin_13
#define     FSMC_PIN_SOURCE_D10                   GPIO_PinSource13
#define     FSMC_PIN_D11                          GPIO_Pin_14
#define     FSMC_PIN_SOURCE_D11                   GPIO_PinSource14
#define     FSMC_PIN_D12                          GPIO_Pin_15
#define     FSMC_PIN_SOURCE_D12                   GPIO_PinSource15

#define     FSMC_PINS_D4_12                      (FSMC_PIN_D4   | \
												  FSMC_PIN_D5   | \
												  FSMC_PIN_D6   | \
												  FSMC_PIN_D7   | \
												  FSMC_PIN_D8   | \
												  FSMC_PIN_D9   | \
												  FSMC_PIN_D10  | \
												  FSMC_PIN_D11  | \
												  FSMC_PIN_D12)

/*----------------------------    D13-D15引脚宏定义   --------------------------*/

#define     FSMC_GPIO_PORT_D13_15                 GPIOD
#define     FSMC_GPIO_PORT_RCC_D13_15             RCC_AHB1Periph_GPIOD

#define     FSMC_PIN_D13                          GPIO_Pin_8
#define     FSMC_PIN_SOURCE_D13                   GPIO_PinSource8
#define     FSMC_PIN_D14                          GPIO_Pin_9
#define     FSMC_PIN_SOURCE_D14                   GPIO_PinSource9
#define     FSMC_PIN_D15                          GPIO_Pin_10
#define     FSMC_PIN_SOURCE_D15                   GPIO_PinSource10


#define     FSMC_PINS_D13_15                     (FSMC_PIN_D13   | \
												  FSMC_PIN_D14   | \
												  FSMC_PIN_D15)

/*----------------------------    公共控制引脚，读写   --------------------------*/


#define    FSMC_GPIO_PORT_NWE                     GPIOD
#define    FSMC_GPIO_PORT_RCC_NWE                 RCC_AHB1Periph_GPIOD
#define    FSMC_PIN_NWE                           GPIO_Pin_5  
#define    FSMC_PIN_SOURCE_NWE                    GPIO_PinSource5

#define    FSMC_GPIO_PORT_NOE                     GPIOD
#define    FSMC_GPIO_PORT_RCC_NOE                 RCC_AHB1Periph_GPIOD
#define    FSMC_PIN_NOE                           GPIO_Pin_4  
#define    FSMC_PIN_SOURCE_NOE                    GPIO_PinSource4





/*
*********************************************************************************************************
*                                               FUNCTION
*********************************************************************************************************
*/

void         BSP_FSMC_COMMON_Init(void);



#endif        /* __BSP_FSMC_H */


