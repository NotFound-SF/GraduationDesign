
#ifndef __BSP_MOTOR_H
#define	__BSP_MOTOR_H

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


// PC6 

#define      MOTOR_IN1_PIN             GPIO_Pin_6
#define      MOTOR_IN1_PORT            GPIOC
#define      MOTOR_IN1_GPIO_CLK        RCC_AHB1Periph_GPIOC  
#define      MOTOR_IN1_HIGH()       do { MOTOR_IN1_PORT->BSRRL = MOTOR_IN1_PIN; } while(0)                   //输出1
#define      MOTOR_IN1_LOW()        do { MOTOR_IN1_PORT->BSRRH = MOTOR_IN1_PIN; } while(0)   


// PC7  

#define      MOTOR_IN2_PIN             GPIO_Pin_7
#define      MOTOR_IN2_PORT            GPIOC
#define      MOTOR_IN2_GPIO_CLK        RCC_AHB1Periph_GPIOC  
#define      MOTOR_IN2_HIGH()       do { MOTOR_IN2_PORT->BSRRL = MOTOR_IN2_PIN; } while(0)                   //输出1
#define      MOTOR_IN2_LOW()        do { MOTOR_IN2_PORT->BSRRH = MOTOR_IN2_PIN; } while(0)   


// PC8  

#define      MOTOR_IN3_PIN             GPIO_Pin_8
#define      MOTOR_IN3_PORT            GPIOC
#define      MOTOR_IN3_GPIO_CLK        RCC_AHB1Periph_GPIOC  
#define      MOTOR_IN3_HIGH()       do { MOTOR_IN3_PORT->BSRRL = MOTOR_IN3_PIN; } while(0)                   //输出1
#define      MOTOR_IN3_LOW()        do { MOTOR_IN3_PORT->BSRRH = MOTOR_IN3_PIN; } while(0)     


// PC9  

#define      MOTOR_IN4_PIN             GPIO_Pin_9
#define      MOTOR_IN4_PORT            GPIOC
#define      MOTOR_IN4_GPIO_CLK        RCC_AHB1Periph_GPIOC  
#define      MOTOR_IN4_HIGH()       do { MOTOR_IN4_PORT->BSRRL = MOTOR_IN4_PIN; } while(0)                   //输出1
#define      MOTOR_IN4_LOW()        do { MOTOR_IN4_PORT->BSRRH = MOTOR_IN4_PIN; } while(0)   



// 转轴针对我们的方向

typedef enum {                                                      
	MOTOR_DIR_RIGHT = 0x00,                                                                                     
	MOTOR_DIR_LIFT  = 0x01                                                                                                                                   
}MOTOR_DIR_X; 



/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

void         BSP_Turn_Motor(uint16_t angle, MOTOR_DIR_X dir);
CPU_BOOLEAN  BSP_MOTOR_Init(void);


#endif /* __BSP_MOTOR_H */


