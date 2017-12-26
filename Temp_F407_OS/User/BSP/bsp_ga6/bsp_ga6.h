
#ifndef    __BSP_GA6_H
#define    __BSP_GA6_H


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f4xx.h"
#include "bsp_uart.h"  


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define      GA6_UART_PORT                      BSP_UART_ID_2
#define      GA6_UART_BAUD                      115200
#define      RES_BUFF_LEN                       26


typedef enum {                                                      /*----------------- I2C工作模式 -------------*/
	REG_NORMAL  = 0x01,                                  //注册成功
	REG_NO_SING = 0x02,                                  //无信号
	REG_NO_CARD = 0x03,                                  //无SIM卡
	REG_UNKNOWN = 0x04,                                  //未知情况
	REG_ROAM    = 0x05                                   //处于漫游状态
}GA6_Reg_Status;




/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

CPU_BOOLEAN         BSP_GA6_Init(void);
uint8_t             BSP_GA6_GetSigQu(void);
GA6_Reg_Status      BSP_GA6_GetRegSt(void);
CPU_BOOLEAN         BSP_GA6_SenTextMSG(char *num, char *str);
CPU_BOOLEAN         BSP_GA6_CheckCall(void);
CPU_BOOLEAN         BSP_GA6_SenTextMSG(char *num, char *str);
CPU_BOOLEAN         BSP_GA6_CheckEx(void);
CPU_BOOLEAN         BSP_GA6_CallHang(void);





#endif   /* __BSP_GA6_H */










