
#ifndef   __BSP_LED_H
#define   __BSP_LED_H

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "stm32f4xx.h"


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define      LED1_PIN             	 GPIO_Pin_3
#define      LED2_PIN			     GPIO_Pin_4
#define      LED3_PIN			     GPIO_Pin_9

    

#define      LED12_GPIO_CLK               RCC_AHB1Periph_GPIOE              /* 端口时钟                  */
#define      LED12_GPIO_PORT              GPIOE

#define      LED3_GPIO_CLK                RCC_AHB1Periph_GPIOG              /* 端口时钟                  */
#define      LED3_GPIO_PORT               GPIOG

      



/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         BSP_LED_Init                (void);

void         BSP_LED_On                  (uint8_t     led);

void         BSP_LED_Off                 (uint8_t     led);

void         BSP_LED_Toggle              (uint8_t     led);



#endif    //__BSP_LED_H



