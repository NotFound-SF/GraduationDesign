
#include "bsp_spi.h"
#include "bsp_w25q128.h"


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static          OS_SEM                  SemLock;                // 用于FLASH独占访问


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void  BSP_FLASH_WriteEnable(void);
static void  BSP_FLASH_WaitForWriteEnd(void);
static void  BSP_FLASH_PageWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_FLASH_Init()
*
* Description : 初始板载的FLASH芯片的底层SPI接口
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_FLASH_Init(void) 
{
	OS_ERR err;
	
	// 创建端口锁定信号量
	
	OSSemCreate((OS_SEM    *)& SemLock,            
				(CPU_CHAR  *)  "SPI_FLASH Lock", 
				(OS_SEM_CTR )  1,
				(OS_ERR    *)& err);
	
	// 初始化SPI底层接口
	
	BSP_SPIx_Init();
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_ReadID()
*
* Description : 读取板载FLASH的ID值
*
* Argument(s) : none
*
* Return(s)   : 板载FLASH的ID值
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/


uint32_t BSP_FLASH_ReadID(void)
{
	OS_ERR   err;
	uint8_t  Temp[3];
	
	// 锁定FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // 永久等待                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // 阻塞等待                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
	
	SPI_FLASH_CS_LOW();                                        // 拉低片选线
	
	SPI_FLASH_SendByte(W25X_JedecDeviceID);                    // 发送指令
    SPI_FLASH_ReadBuff(Temp, 3);                               // 连续读可以提高效率

	SPI_FLASH_CS_HIGH();                                       // 拉高片选

	// 释放SPI端口
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
			  
    return ((uint32_t)Temp[0] << 16| (uint32_t)Temp[1] << 8| (uint32_t)Temp[2]);
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_ReadDeviceID()
*
* Description : 读取设备地址
*
* Argument(s) : none
*
* Return(s)   : 32位的设备ID
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/


uint32_t BSP_FLASH_ReadDeviceID(void)
{
	OS_ERR    err;
	uint32_t  Temp = 0;
	
	// 锁定FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // 永久等待                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // 阻塞等待                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
    SPI_FLASH_CS_LOW();

    // 发送读取设备ID指令，并且读取ID
    SPI_FLASH_SendByte(W25X_DeviceID);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
	Temp = BSP_SPI_ReadByte();

    SPI_FLASH_CS_HIGH();
  
   // 释放端口
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);

  return Temp;
}



/*
*********************************************************************************************************
*                                             BSP_FLASH_SectorErase()
*
* Description : 扇区擦除函数
*
* Argument(s) : SectorAddr : 扇区地址
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/

void BSP_FLASH_SectorErase(uint32_t SectorAddr)
{
	OS_ERR   err;
	
	// 锁定FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // 永久等待                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // 阻塞等待                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
	
	BSP_FLASH_WriteEnable();
	BSP_FLASH_WaitForWriteEnd();
	
	SPI_FLASH_CS_LOW();
  
	// 发送扇区擦除指令
	
	SPI_FLASH_SendByte(W25X_SectorErase);
	
	// 发送要擦除的扇区地址
	
	SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(SectorAddr & 0xFF);
  
	SPI_FLASH_CS_HIGH();
 
	// 等待擦除结束
	
	BSP_FLASH_WaitForWriteEnd();
	
	// 释放端口
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_BufferWrite()
*
* Description : FLASH 连续写入
*
* Argument(s) : pBuffer         :要写入的缓冲区
*               WriteAddr       :要写入的地址
*               NumByteToWrite  :要写入的字节数
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/


void BSP_FLASH_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	OS_ERR    err;
    uint16_t  NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
	// 锁定FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // 永久等待                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // 阻塞等待                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	

	Addr = WriteAddr % SPI_FLASH_PageSize;
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    if (Addr == 0) {                                                              // 表明地址页对齐的
		if (NumOfPage == 0) {
			BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
		} else {                                                                  // 表示有完整页
		
			// 写完整的页
			
			while (NumOfPage--) {                                                 // 写完整页
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
		}
	} else {                                                                      // 表明地址未页对齐的
		if (NumOfPage == 0) {                                                     // 没有完整页
			if (NumOfSingle > count) {
				temp = NumOfSingle - count;
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, count);
				WriteAddr +=  count;
				pBuffer += count;
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, temp);
			} else {
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
			}
		} else {                                                                  // 有完整页
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			BSP_FLASH_PageWrite(pBuffer, WriteAddr, count);
			WriteAddr +=  count;
			pBuffer += count;

			// 写完整的页
			
			while (NumOfPage--) {                      
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			if (NumOfSingle != 0) {
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
  
	// 释放端口
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err); 
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_BufferRead()
*
* Description : FLASH 连续读入
*
* Argument(s) : pBuffer         :要写入的缓冲区
*               WriteAddr       :要读的地址
*               NumByteToWrite  :要读的字节数
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 
*********************************************************************************************************
*/


void BSP_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	OS_ERR   err;
	
	// 锁定FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // 永久等待                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // 阻塞等待                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
    SPI_FLASH_CS_LOW();

	// 发送读指令，并且发送读地址
	
    SPI_FLASH_SendByte(W25X_ReadData);
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

	// 读取缓冲区
	
	SPI_FLASH_ReadBuff(pBuffer, NumByteToRead);

	SPI_FLASH_CS_HIGH();
  
	// 释放端口
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
  
}




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_FLASH_WriteEnable()
*
* Description : Flash写使能
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_PageWrite(), BSP_FLASH_SectorErase()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_FLASH_WriteEnable(void)
{
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_WriteEnable);

	SPI_FLASH_CS_HIGH();
}



 

/*
*********************************************************************************************************
*                                             BSP_FLASH_WaitForWriteEnd()
*
* Description : 芯片忙等待，擦除，写入都是耗时的，该函数是等待Flash芯片空闲
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_SectorErase(), BSP_FLASH_PageWrite()
*
* Note(s)     : 该函数会调用延时函数
*********************************************************************************************************
*/


static void BSP_FLASH_WaitForWriteEnd(void)
{
	OS_ERR   err;
	uint8_t  FLASH_Status = 0;
	
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_ReadStatusReg);                      // 读取状态机传奇指令
	
	do {
		// 延时1ms,因为FLASH页编程时间典型值是0.7ms
		
		OSTimeDly((OS_TICK )  1, 
				  (OS_OPT  )  OS_OPT_TIME_DLY, 
				  (OS_ERR *)& err);
		
		FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);           //读取状态寄存器
		
	} while (SET == (FLASH_Status & WIP_Flag));
	
	SPI_FLASH_CS_HIGH();
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_PageWrite()
*
* Description : FLASH 页写入函数
*
* Argument(s) : pBuffer         :要写入的缓冲区
*               WriteAddr       :要写入的地址
*               NumByteToWrite  :要写入的字节数
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_BufferWrite()
*
* Note(s)     : none
*********************************************************************************************************
*/

static void BSP_FLASH_PageWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	BSP_FLASH_WriteEnable();

	SPI_FLASH_CS_LOW();
	
	// 发送页编程指令
	
	SPI_FLASH_SendByte(W25X_PageProgram);
	  
	// 发送页地址
	
	SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(WriteAddr & 0xFF);

	// 判断写入时候超过一页
	
	if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
	}
	
	// 将缓冲区数据写入
	
	SPI_FLASH_WriteBuff(pBuffer, NumByteToWrite);              
	
	SPI_FLASH_CS_HIGH();

	// 等待也写入结束
	
	BSP_FLASH_WaitForWriteEnd();
}








