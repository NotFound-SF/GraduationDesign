
#ifndef     __BSP_W25Q128_H
#define     __BSP_W25Q128_H

#include "bsp_spi.h"
#include "diskio.h"


// 该FLASH的一个扇区大小为4096个Bytes,最小擦除单位是扇区

//#define  sFLASH_ID                        0xEF3015     //W25X16
#define  sFLASH_ID                        0xEF4015	 //W25Q16
//#define  sFLASH_ID                        0XEF4017     //W25Q64
//#define  sFLASH_ID                        0XEF4018     //W25Q128

/* Private typedef -----------------------------------------------------------*/

#define SPI_FLASH_PageSize                      256
#define SPI_FLASH_PerWritePageSize              256

/* Private define ------------------------------------------------------------*/
#define W25X_WriteEnable		                0x06 
#define W25X_WriteDisable		                0x04 
#define W25X_ReadStatusReg		                0x05 
#define W25X_WriteStatusReg		                0x01 
#define W25X_ReadData			                0x03 
#define W25X_FastReadData		                0x0B 
#define W25X_FastReadDual		                0x3B 
#define W25X_PageProgram		                0x02 
#define W25X_BlockErase			                0xD8 
#define W25X_SectorErase		                0x20 
#define W25X_ChipErase			                0xC7 
#define W25X_PowerDown			                0xB9 
#define W25X_ReleasePowerDown	                0xAB 
#define W25X_DeviceID			                0xAB 
#define W25X_ManufactDeviceID   	            0x90 
#define W25X_JedecDeviceID		                0x9F 

#define WIP_Flag                                0x01  /* Write In Progress (WIP) flag */


/*
*********************************************************************************************************
*                                              底层接口函数宏
*********************************************************************************************************
*/


#define     Dummy_Byte                         0xFF

// 底层接口函数宏定义

#define     SPI_FLASH_CS_HIGH                  BSP_SPIx_CS_DISABLE    
#define     SPI_FLASH_CS_LOW                   BSP_SPIx_CS_ENABLE
#define     SPI_FLASH_ReadByte                 BSP_SPI_ReadByte
#define     SPI_FLASH_SendByte                 BSP_SPI_WriteByte
#define     SPI_FLASH_ReadBuff                 BSP_SPI_ReadOnly
#define     SPI_FLASH_WriteBuff                BSP_SPI_WriteOnly      




/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void      BSP_FLASH_Init(void);
void      BSP_Flash_PowerDown(void); 
void      BSP_Flash_WAKEUP(void); 
void      BSP_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, uint32_t NumByteToRead);
void      BSP_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void      BSP_FLASH_SectorErase(u32 SectorAddr);
void      BSP_FLASH_BulkErase(void);
uint32_t  BSP_FLASH_ReadID(void);
uint32_t  BSP_FLASH_ReadDeviceID(void);



/*
*********************************************************************************************************
*                                          FatFs文件系统接口函数
*********************************************************************************************************
*/

DSTATUS   TM_FATFS_FLASH_SPI_disk_initialize(void);
DSTATUS   TM_FATFS_FLASH_SPI_disk_status(void) ;
DRESULT   TM_FATFS_FLASH_SPI_disk_ioctl(BYTE cmd, char *buff) ;
DRESULT   TM_FATFS_FLASH_SPI_disk_read(BYTE *buff, DWORD sector, UINT count) ;
DRESULT   TM_FATFS_FLASH_SPI_disk_write(BYTE *buff, DWORD sector, UINT count) ;





#endif     /* __BSP_W25Q128_H */


