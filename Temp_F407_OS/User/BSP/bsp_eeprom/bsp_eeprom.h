
#ifndef    __BSP_EEPROM_H
#define    __BSP_EEPROM_H


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "includes.h"
#include "stm32f4xx.h"
#include "bsp_iic.h"  


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define      AT_24C16
#define      EEPROM_WRITE_DLY_MS                      10                    //写数据时间间隔ms

#define      EEPROM_DEV_ADDR                          0xA0                  //A0,A1,A2未用作地址端口对应位必须清零 
                                                                                                
 
#define      EEPROM_I2C_PORT                          BSP_I2C_ID_1
#define      EEPROM_LOCK_TIME                         0


#if defined(AT_24C01) || defined(AT_24C02)
    #define  PAGE_SIZE           0x08   
#elif defined(AT_24C04) || defined(AT_24C08) || defined(AT_24C16)
    #define  PAGE_SIZE           0x10  
#else
#endif


#if defined(AT_24C01)
    #define  EEPROM_ADDR_MAX      127
	#define  EEPROM_LEN_MAX       128
#elif defined(AT_24C02)
    #define  EEPROM_ADDR_MAX      255
	#define  EEPROM_LEN_MAX       256
#elif defined(AT_24C04)
    #define  EEPROM_ADDR_MAX      511
	#define  EEPROM_LEN_MAX       512
#elif defined(AT_24C08)
    #define  EEPROM_ADDR_MAX      1023
	#define  EEPROM_LEN_MAX       1024
#elif defined(AT_24C16)
    #define  EEPROM_ADDR_MAX      2047
	#define  EEPROM_LEN_MAX       2048
#else
#endif


/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

CPU_BOOLEAN         BSP_EEPROM_Init  (void);
CPU_BOOLEAN         BSP_EEPROM_Rd    (CPU_INT08U *ptr, CPU_INT16U len, CPU_INT16U addr);
CPU_BOOLEAN         BSP_EEPROM_Wr    (CPU_INT08U *ptr, CPU_INT16U len, CPU_INT16U addr);



#endif   /* __BSP_EEPROM_H */






