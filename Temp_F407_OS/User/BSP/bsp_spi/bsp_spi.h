
#ifndef   __BSP_SPI_H
#define   __BSP_SPI_H

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

#define      BSP_SPIx_PORT                SPI1
#define      BSP_SPIx_GPIO_AF             GPIO_AF_SPI1
#define      BSP_SPIx_RCC                 RCC_APB2Periph_SPI1
#define      BSP_SPIx_RCC_CMD             RCC_APB2PeriphClockCmd
#define      BSP_SPIx_IRQ                 SPI1_IRQn
#define      BSP_SPIx_IRQHandler          SPI1_IRQHandler
#define      BSP_SPIx_NVIC_PRE_PRIO       1                                      // 抢占优先级
#define      BSP_SPIx_NVIC_SUB_PRIO       1                                      // 子优先级  



#define      BSP_SPI_NOUSE                0xFF

#if 0

// PB14  CS
#define      BSP_SPIx_CS_PIN               GPIO_Pin_14
#define      BSP_SPIx_CS_SOURCE            GPIO_PinSource14
#define      BSP_SPIx_CS_PORT              GPIOB
#define      BSP_SPIx_CS_GPIO_CLK          RCC_AHB1Periph_GPIOB  
#define      BSP_SPIx_CS_DISABLE()      do { BSP_SPIx_CS_PORT->BSRRL = BSP_SPIx_CS_PIN; } while(0)                   //输出1
#define      BSP_SPIx_CS_ENABLE()       do { BSP_SPIx_CS_PORT->BSRRH = BSP_SPIx_CS_PIN; } while(0)   

#endif


#if 1
// PG8  CS
#define      BSP_SPIx_CS_PIN              GPIO_Pin_8
#define      BSP_SPIx_CS_SOURCE           GPIO_PinSource8
#define      BSP_SPIx_CS_PORT             GPIOG
#define      BSP_SPIx_CS_GPIO_CLK         RCC_AHB1Periph_GPIOG  
#define      BSP_SPIx_CS_DISABLE()     do { BSP_SPIx_CS_PORT->BSRRL = BSP_SPIx_CS_PIN; } while(0)                   //输出1
#define      BSP_SPIx_CS_ENABLE()      do { BSP_SPIx_CS_PORT->BSRRH = BSP_SPIx_CS_PIN; } while(0)   

#endif


// PB3 CLK
#define      BSP_SPIx_CLK_PIN              GPIO_Pin_3
#define      BSP_SPIx_CLK_PORT             GPIOB
#define      BSP_SPIx_CLK_SOURCE           GPIO_PinSource3
#define      BSP_SPIx_CLK_GPIO_CLK         RCC_AHB1Periph_GPIOB  


// PB4  MISO
#define      BSP_SPIx_MISO_PIN             GPIO_Pin_4
#define      BSP_SPIx_MISO_SOURCE          GPIO_PinSource4
#define      BSP_SPIx_MISO_PORT            GPIOB
#define      BSP_SPIx_MISO_GPIO_CLK        RCC_AHB1Periph_GPIOB


// PB5  MOSI
#define      BSP_SPIx_MOSI_PIN             GPIO_Pin_5
#define      BSP_SPIx_MOSI_SOURCE          GPIO_PinSource5
#define      BSP_SPIx_MOSI_PORT            GPIOB
#define      BSP_SPIx_MOSI_GPIO_CLK        RCC_AHB1Periph_GPIOB  







/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

void       BSP_SPIx_Init(void);
uint8_t    BSP_SPI_ReadByte(void);
uint8_t    BSP_SPI_WriteByte(uint8_t dat);
void       BSP_SPI_WriteOnly(uint8_t *WriteBuff, uint16_t BuffLen);
void       BSP_SPI_ReadOnly(uint8_t *ReadBuff, uint16_t BuffLen);
void       BSP_SPI_ReadWrite(uint8_t *WrBuff, uint8_t *ReadBuff, uint16_t BuffLen);

#endif  /* __BSP_SPI_H */

