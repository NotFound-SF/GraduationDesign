

#ifndef   __BSP_UART_H
#define   __BSP_UART_H

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

#define      BSP_SER_PRINTF_STR_BUF_SIZE         80u                        //Printf缓冲区大小(Byte)
#define      BSP_UAER_LOCK_TIME                  0                          //读写信号量永久等待
#define      BSP_UART_NBR_MAX                    6                          //UART端口数

#define      BSP_UART_NVIC_SUB_PRIO              1                          //UART子优先级
#define      BSP_UART_NVIC_PRE_PRIO              1                          //UART抢占优先级

#define      BSP_UART_ID_1                       0
#define      BSP_UART_ID_2                       1
#define      BSP_UART_ID_3                       2
#define      BSP_UART_ID_4                       3
#define      BSP_UART_ID_5                       4
#define      BSP_UART_ID_6                       5

#define      UART_IDEL_RXN_MASK                  (DEF_BIT_04|DEF_BIT_05)

/*
*********************************************************************************************************
*                                               USART1
*********************************************************************************************************
*/
#define      UART1_GPIO_PORT_TX                  GPIOA                           
#define      UART1_TX_PIN             	    	 GPIO_Pin_9
#define      UART1_TX_PIN_SOURCE                 GPIO_PinSource9
#define      UART1_GPIO_CLK_TX                   RCC_AHB1Periph_GPIOA

#define      UART1_GPIO_PORT_RX                  GPIOA
#define      UART1_RX_PIN             		     GPIO_Pin_10
#define      UART1_RX_PIN_SOURCE                 GPIO_PinSource10
#define      UART1_GPIO_CLK_RX                   RCC_AHB1Periph_GPIOA

#define      UART1_DEV_CLK                       RCC_APB2Periph_USART1
#define      UART1_CLK_CMD                       RCC_APB2PeriphClockCmd
#define      BSP_UART1_IRQ                       USART1_IRQn
#define      BSP_UART1_Handler                   USART1_IRQHandler


/*
*********************************************************************************************************
*                                               USART2
*********************************************************************************************************
*/
#define      UART2_GPIO_PORT_TX                  GPIOA
#define      UART2_TX_PIN             	    	 GPIO_Pin_2
#define      UART2_TX_PIN_SOURCE                 GPIO_PinSource2
#define      UART2_GPIO_CLK_TX                   RCC_AHB1Periph_GPIOA

#define      UART2_GPIO_PORT_RX                  GPIOA
#define      UART2_RX_PIN             		     GPIO_Pin_3
#define      UART2_RX_PIN_SOURCE                 GPIO_PinSource3
#define      UART2_GPIO_CLK_RX                   RCC_AHB1Periph_GPIOA

#define      UART2_DEV_CLK                       RCC_APB1Periph_USART2
#define      UART2_CLK_CMD                       RCC_APB1PeriphClockCmd
#define      BSP_UART2_IRQ                       USART2_IRQn
#define      BSP_UART2_Handler                   USART2_IRQHandler


/*
*********************************************************************************************************
*                                               USART3
*********************************************************************************************************
*/
#define      UART3_GPIO_PORT_TX                  GPIOB
#define      UART3_TX_PIN             	    	 GPIO_Pin_10
#define      UART3_TX_PIN_SOURCE                 GPIO_PinSource10
#define      UART3_GPIO_CLK_TX                   RCC_AHB1Periph_GPIOB

#define      UART3_GPIO_PORT_RX                  GPIOB
#define      UART3_RX_PIN             		     GPIO_Pin_11
#define      UART3_RX_PIN_SOURCE                 GPIO_PinSource11
#define      UART3_GPIO_CLK_RX                   RCC_AHB1Periph_GPIOB

#define      UART3_DEV_CLK                       RCC_APB1Periph_USART3
#define      UART3_CLK_CMD                       RCC_APB1PeriphClockCmd
#define      BSP_UART3_IRQ                       USART3_IRQn
#define      BSP_UART3_Handler                   USART3_IRQHandler


/*
*********************************************************************************************************
*                                               USART4
*********************************************************************************************************
*/
#define      UART4_GPIO_PORT_TX                  GPIOA
#define      UART4_TX_PIN             	    	 GPIO_Pin_0
#define      UART4_TX_PIN_SOURCE                 GPIO_PinSource0
#define      UART4_GPIO_CLK_TX                   RCC_AHB1Periph_GPIOA

#define      UART4_GPIO_PORT_RX                  GPIOA
#define      UART4_RX_PIN             		     GPIO_Pin_1
#define      UART4_RX_PIN_SOURCE                 GPIO_PinSource1
#define      UART4_GPIO_CLK_RX                   RCC_AHB1Periph_GPIOA

#define      UART4_DEV_CLK                       RCC_APB1Periph_UART4
#define      UART4_CLK_CMD                       RCC_APB1PeriphClockCmd
#define      BSP_UART4_IRQ                       UART4_IRQn
#define      BSP_UART4_Handler                   UART4_IRQHandler


/*
*********************************************************************************************************
*                                               USART5
*********************************************************************************************************
*/
#define      UART5_GPIO_PORT_TX                  GPIOC
#define      UART5_TX_PIN             	    	 GPIO_Pin_12
#define      UART5_TX_PIN_SOURCE                 GPIO_PinSource12
#define      UART5_GPIO_CLK_TX                   RCC_AHB1Periph_GPIOC

#define      UART5_GPIO_PORT_RX                  GPIOD
#define      UART5_RX_PIN             		     GPIO_Pin_2
#define      UART5_RX_PIN_SOURCE                 GPIO_PinSource2
#define      UART5_GPIO_CLK_RX                   RCC_AHB1Periph_GPIOD

#define      UART5_DEV_CLK                       RCC_APB1Periph_UART5
#define      UART5_CLK_CMD                       RCC_APB1PeriphClockCmd
#define      BSP_UART5_IRQ                       UART5_IRQn
#define      BSP_UART5_Handler                   UART5_IRQHandler



/*
*********************************************************************************************************
*                                               USART6
*********************************************************************************************************
*/
#define      UART6_GPIO_PORT_TX                  GPIOC
#define      UART6_TX_PIN             	    	 GPIO_Pin_6
#define      UART6_TX_PIN_SOURCE                 GPIO_PinSource6
#define      UART6_GPIO_CLK_TX                   RCC_AHB1Periph_GPIOC

#define      UART6_GPIO_PORT_RX                  GPIOC
#define      UART6_RX_PIN             		     GPIO_Pin_7
#define      UART6_RX_PIN_SOURCE                 GPIO_PinSource7
#define      UART6_GPIO_CLK_RX                   RCC_AHB1Periph_GPIOC

#define      UART6_DEV_CLK                       RCC_APB2Periph_USART6
#define      UART6_CLK_CMD                       RCC_APB2PeriphClockCmd
#define      BSP_UART6_IRQ                       USART6_IRQn
#define      BSP_UART6_Handler                   USART6_IRQHandler



/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void         BSP_UART_Init   (CPU_INT08U uart_id, CPU_INT32U uart_baud);
CPU_INT16U   BSP_UART_RdData (CPU_INT08U uart_id, CPU_INT08U *data, CPU_INT16U len, OS_TICK timeOut);
CPU_INT16U   BSP_UART_WrData (CPU_INT08U uart_id, CPU_INT08U *data, CPU_INT16U len);
void         BSP_UART_WrByte (CPU_INT08U uart_id, CPU_INT08U data);
CPU_INT08U   BSP_UART_RdByte (CPU_INT08U uart_id, OS_TICK timeOut);
void         BSP_UART_WrStr  (CPU_INT08U uart_id, CPU_CHAR *data);
void         BSP_UART_Printf (CPU_INT08U uart_id, CPU_CHAR  *format, ...);


#endif                                                         /* __BSP_UART_H */



