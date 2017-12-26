
#ifndef   __BSP_ADC_H
#define   __BSP_ADC_H


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

// 通道个数

#define         DAC_CH_NUM                              3                                 //通道个数

// 实质是CH1是ADC1,2,3的CH0    PA0

#define         DAC_CH1_GPIO_PORT                       GPIOA
#define         DAC_CH1_PIN                             GPIO_Pin_0
#define         DAC_CH1_PIN_SOURCE                      GPIO_PinSource0
#define         DAC_CH1_GPIO_RCC                        RCC_AHB1Periph_GPIOA
#define         DAC_CH1                                 ADC_Channel_0

// 实质是CH1是ADC1,2的CH1      PA1

#define         DAC_CH2_GPIO_PORT                       GPIOA
#define         DAC_CH2_PIN                             GPIO_Pin_1
#define         DAC_CH2_PIN_SOURCE                      GPIO_PinSource1
#define         DAC_CH2_GPIO_RCC                        RCC_AHB1Periph_GPIOA
#define         DAC_CH2                                 ADC_Channel_1

// 实质是CH1是ADC1,2的CH4      PA4

#define         DAC_CH3_GPIO_PORT                       GPIOA
#define         DAC_CH3_PIN                             GPIO_Pin_4
#define         DAC_CH3_PIN_SOURCE                      GPIO_PinSource4
#define         DAC_CH3_GPIO_RCC                        RCC_AHB1Periph_GPIOA
#define         DAC_CH3                                 ADC_Channel_4


// 通道编号，用于获取数据

typedef enum {                                                      
	BSP_ADC_CH1  = 0x00,                                                                                     
	BSP_ADC_CH2  = 0x01,                                                                             
	BSP_ADC_CH3  = 0x02                                                        
}BSP_ADC_CHx; 


// ADC外设相关宏定义

#define         ADC_PORT                                ADC1
#define 		ADC_CLK                                 RCC_APB2Periph_ADC1
#define         ADC_RCC_CMD                             RCC_APB2PeriphClockCmd
#define         ADC_DR_ADDR                             ((uint32_t)ADC1+0x4c)           

#define         ADC_DMA_CLK                             RCC_AHB1Periph_DMA2
#define         ADC_DMA_CHANNEL                         DMA_Channel_0
#define         ADC_DMA_STREAM                          DMA2_Stream0


/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

void                BSP_ADC_Init(void);
uint16_t            BSP_ADC_GetDat(BSP_ADC_CHx CHx);



#endif   /* __BSP_ADC_H */





