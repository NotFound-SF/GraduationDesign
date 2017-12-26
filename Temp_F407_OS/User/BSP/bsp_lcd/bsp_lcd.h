
#ifndef     __BSP_LCD_H
#define     __BSP_LCD_H

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

#define    BSP_LCD_Bank                              FSMC_Bank1_NORSRAM4

// 使用的是A12作为命令数据

#define    BSP_LCD_DAT_BASE                          (uint32_t)0x6C002000
#define    BSP_LCD_CMD_BASE                          (uint32_t)0x6C000000

#define    BSP_LCD_CMD                               (*(__IO uint16_t *)BSP_LCD_CMD_BASE)  
#define    BSP_LCD_DAT                               (*(__IO uint16_t *)BSP_LCD_DAT_BASE)

/*
*********************************************************************************************************
*                                               颜色定义
*********************************************************************************************************
*/

#define      WHITE		 		           0xFFFF	   //白色
#define      BLACK                         0x0000	   //黑色 
#define      GREY                          0xF7DE	   //灰色 
#define      BLUE                          0x001F	   //蓝色 
#define      BLUE2                         0x051F	   //浅蓝色 
#define      RED                           0xF800	   //红色 
#define      MAGENTA                       0xF81F	   //红紫色，洋红色 
#define      GREEN                         0x07E0	   //绿色 
#define      CYAN                          0x7FFF	   //蓝绿色，青色 
#define      YELLOW                        0xFFE0	   //黄色 
#define      BRED                          0xF81F
#define      GRED                          0xFFE0
#define      GBLUE                         0x07FF


/*
*********************************************************************************************************
*                                               命令定义
*********************************************************************************************************
*/
#define      CMD_SetCoordinateX		 		         0x2A	     //设置X坐标
#define      CMD_SetCoordinateY		 		         0x2B	     //设置Y坐标
#define      CMD_SetPixel		 		             0x2C	     //填充像素


/***************************** ILI934 显示区域的起始坐标和总行列数 ***************************/
#define      ILI9341_DispWindow_X_Star		         0           //起始点的X坐标
#define      ILI9341_DispWindow_Y_Star		         0           //起始点的Y坐标

#define 	 ILI9341_LESS_PIXEL	  		        	 240	     //液晶屏较短方向的像素宽度
#define 	 ILI9341_MORE_PIXEL	 		        	 320	     //液晶屏较长方向的像素宽度
#define      ILI9341_ALL_PIXEL                       76800       //像素点个数 > 65535



/*
*********************************************************************************************************
*                                               LCD控制引脚定义
*********************************************************************************************************
*/

/*-----------------------------  移植时需要修改该引脚定义  ------------------------*/
#define    LCD_GPIO_PORT_NCE                     GPIOG
#define    LCD_GPIO_PORT_RCC_NCE                 RCC_AHB1Periph_GPIOG
#define    LCD_PIN_NCE                           GPIO_Pin_12  
#define    LCD_PIN_SOURCE_NCE                    GPIO_PinSource12

#define    LCD_GPIO_PORT_BL                      GPIOF
#define    LCD_GPIO_PORT_RCC_BL                  RCC_AHB1Periph_GPIOF
#define    LCD_PIN_BL                            GPIO_Pin_10  
#define    LCD_PIN_SOURCE_BL                     GPIO_PinSource10

// LCD背光控制

#define    BSP_LCD_BL_ON()                       (LCD_GPIO_PORT_BL->BSRRL = LCD_PIN_BL)
#define    BSP_LCD_BL_OFF()                      (LCD_GPIO_PORT_BL->BSRRH = LCD_PIN_BL)


// DMA相关

#define         LCD_DMA_EN                       1          // 大于0表示使能DMA

#if  LCD_DMA_EN > 0

#define         LCD_DMA_ID                       DMA2
#define         LCD_DMA_CTC_MASK                 DEF_BIT_11
#define         LCD_DMA_FTC_MASK                 DEF_BIT_11
#define         LCD_IFCR                         LIFCR 
#define         LCD_ISR                          LISR 
#define         LCD_DMA_CLK                      RCC_AHB1Periph_DMA2
#define         LCD_DMA_CHANNEL                  DMA_Channel_0
#define         LCD_DMA_STREAM                   DMA2_Stream1

#endif /* LCD_DMA_EN */





/*
*********************************************************************************************************
*                                               FUNCTION
*********************************************************************************************************
*/

void       BSP_LCD_Init       (void);
void       BSP_LCD_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );
void       LCD_Test           (uint16_t *color);



#endif      /* __BSP_LCD_H */











