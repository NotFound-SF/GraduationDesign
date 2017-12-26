

#ifndef    __BSP_18B20_H
#define    __BSP_18B20_H


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
#define      DS18B20_PIN_NUM        15 
#define      DS18B20_MOD_SHIFT      30                                                                         //DS18B20_PIN_NUM*2
                                           
#define      DS18B20_PIN            GPIO_Pin_15

#define      DS18B20_GPIO_PORT      GPIOA
#define      DS18B20_GPIO_CLK       RCC_AHB1Periph_GPIOA                   

#define      DS18B20_PIN_OUT()      do { DS18B20_GPIO_PORT->MODER &= ~((uint32_t)0x03<<DS18B20_MOD_SHIFT);  \
										 DS18B20_GPIO_PORT->MODER |=  (uint32_t)0x01<<DS18B20_MOD_SHIFT;   } while(0)        //配置为通用输出模式01

#define      DS18B20_PIN_IN()       do { DS18B20_GPIO_PORT->MODER &= ~((uint32_t)0x03<<DS18B20_MOD_SHIFT); } while(0)        //配置为输人模式00
										 
#define      DS18B20_PIN_WR_1()     do { DS18B20_GPIO_PORT->BSRRL = DS18B20_PIN; } while(0)                    //输出1
#define      DS18B20_PIN_WR_0()     do { DS18B20_GPIO_PORT->BSRRH = DS18B20_PIN; } while(0)                    //输出0
#define      DS18B20_PIN_RD()	    ((DS18B20_GPIO_PORT->IDR &= DS18B20_PIN) >> DS18B20_PIN_NUM)	           //读取IO脚状态返回0或1			 


// 传感器工作的精度
#define      ACCURACY_MODE_9  

// 温度精度宏 
#define      ACCURACY_9            0x1F    
#define      ACCURACY_10           0x3F        
#define      ACCURACY_11           0x5F        
#define      ACCURACY_12           0x7F     

// 温度转换实际时间
#define      CONVERT_T_9           96   
#define      CONVERT_T_10          188   
#define      CONVERT_T_11          376   
#define      CONVERT_T_12          760   


#if defined(ACCURACY_MODE_9)
    #define  ACCURACY             ACCURACY_9
	#define  CONVERT_T            CONVERT_T_9
#elif defined(ACCURACY_MODE_10)
    #define  ACCURACY             ACCURACY_10
	#define  CONVERT_T            CONVERT_T_10
#elif defined(ACCURACY_MODE_11)
    #define  ACCURACY             ACCURACY_11
	#define  CONVERT_T            CONVERT_T_11
#elif defined(ACCURACY_MODE_12)
    #define  ACCURACY             ACCURACY_12
	#define  CONVERT_T            CONVERT_T_12
#else
#endif



/*
*********************************************************************************************************
*                                               FUCTIONS
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_18B20_Init(void);
CPU_BOOLEAN  BSP_18B20_GetTemp(uint16_t *temp);
uint32_t     BSP_18B20_GetTempFast(void);
float        BSP_18B20_TempTran(uint16_t temp);
										    


#endif     /* __BSP_18B20_H */



