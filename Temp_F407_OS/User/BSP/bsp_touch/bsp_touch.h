
#ifndef __BSP_TOUCH_H
#define	__BSP_TOUCH_H

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


// PF11 PEN

#define      TOUCH_PEN_PIN            GPIO_Pin_11
#define      TOUCH_PEN_PORT           GPIOF
#define      TOUCH_PEN_GPIO_CLK       RCC_AHB1Periph_GPIOF  
#define      TOUCH_PEN_RD()	         ((TOUCH_PEN_PORT->IDR &= GPIO_Pin_11) >> 11)	                       //读取IO脚状态返回0或1

#define      macXPT2046_INT_ActiveLevel	 0



// PB0  CS
#define      TOUCH_CS_PIN              GPIO_Pin_0
#define      TOUCH_CS_PORT             GPIOB
#define      TOUCH_CS_GPIO_CLK         RCC_AHB1Periph_GPIOB  
#define      TOUCH_CS_DISABLE()     do { TOUCH_CS_PORT->BSRRL = TOUCH_CS_PIN; } while(0)                   //输出1
#define      TOUCH_CS_ENABLE()      do { TOUCH_CS_PORT->BSRRH = TOUCH_CS_PIN; } while(0)   



// PB1  MOSI
#define      TOUCH_MOSI_PIN            GPIO_Pin_1
#define      TOUCH_MOSI_PORT           GPIOB
#define      TOUCH_MOSI_GPIO_CLK       RCC_AHB1Periph_GPIOB  
#define      TOUCH_MOSI_1()         do { TOUCH_MOSI_PORT->BSRRL = TOUCH_MOSI_PIN; } while(0)               //输出1
#define      TOUCH_MOSI_0()         do { TOUCH_MOSI_PORT->BSRRH = TOUCH_MOSI_PIN; } while(0)   


// PB2  MISO
#define      TOUCH_MISO_PIN            GPIO_Pin_2
#define      TOUCH_MISO_PORT           GPIOB
#define      TOUCH_MISO_GPIO_CLK       RCC_AHB1Periph_GPIOB  
#define      TOUCH_MISO_RD()	       ((TOUCH_MOSI_PORT->IDR &= TOUCH_MISO_PIN) >> 2)	                    //读取IO脚状态返回0或1


// PA5 CLK
#define      TOUCH_CLK_PIN              GPIO_Pin_5
#define      TOUCH_CLK_PORT             GPIOA
#define      TOUCH_CLK_GPIO_CLK         RCC_AHB1Periph_GPIOA  
#define      TOUCH_CLK_HIGH()        do { TOUCH_CLK_PORT->BSRRL = TOUCH_CLK_PIN; } while(0)                 //输出1
#define      TOUCH_CLK_LOW()         do { TOUCH_CLK_PORT->BSRRH = TOUCH_CLK_PIN; } while(0)   



/*
*********************************************************************************************************
*                                               通道指令
*********************************************************************************************************
*/

#define	            macXPT2046_CHANNEL_X 	                          0x90 	          //通道Y+的选择控制字	
#define	            macXPT2046_CHANNEL_Y 	                          0xd0	          //通道X+的选择控制字




/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

void     XPT2046_Init( void );
uint16_t XPT2046_ReadAdc_Fliter(uint8_t channel); 


#endif /* __BSP_TOUCH_H */





