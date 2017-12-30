
#include "bsp_spi.h"
#include "bsp_w25q128.h"


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static          OS_SEM                  SemLock;                // ����FLASH��ռ����


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
* Description : ��ʼ���ص�FLASHоƬ�ĵײ�SPI�ӿ�
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
	
	// �����˿������ź���
	
	OSSemCreate((OS_SEM    *)& SemLock,            
				(CPU_CHAR  *)  "SPI_FLASH Lock", 
				(OS_SEM_CTR )  1,
				(OS_ERR    *)& err);
	
	// ��ʼ��SPI�ײ�ӿ�
	
	BSP_SPIx_Init();
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_ReadID()
*
* Description : ��ȡ����FLASH��IDֵ
*
* Argument(s) : none
*
* Return(s)   : ����FLASH��IDֵ
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
	
	// ����FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // ���õȴ�                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // �����ȴ�                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
	
	SPI_FLASH_CS_LOW();                                        // ����Ƭѡ��
	
	SPI_FLASH_SendByte(W25X_JedecDeviceID);                    // ����ָ��
    SPI_FLASH_ReadBuff(Temp, 3);                               // �������������Ч��

	SPI_FLASH_CS_HIGH();                                       // ����Ƭѡ

	// �ͷ�SPI�˿�
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
			  
    return ((uint32_t)Temp[0] << 16| (uint32_t)Temp[1] << 8| (uint32_t)Temp[2]);
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_ReadDeviceID()
*
* Description : ��ȡ�豸��ַ
*
* Argument(s) : none
*
* Return(s)   : 32λ���豸ID
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
	
	// ����FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // ���õȴ�                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // �����ȴ�                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
    SPI_FLASH_CS_LOW();

    // ���Ͷ�ȡ�豸IDָ����Ҷ�ȡID
    SPI_FLASH_SendByte(W25X_DeviceID);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
    SPI_FLASH_SendByte(Dummy_Byte);
	Temp = BSP_SPI_ReadByte();

    SPI_FLASH_CS_HIGH();
  
   // �ͷŶ˿�
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);

  return Temp;
}



/*
*********************************************************************************************************
*                                             BSP_FLASH_SectorErase()
*
* Description : ������������
*
* Argument(s) : SectorAddr : ������ַ
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
	
	// ����FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // ���õȴ�                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // �����ȴ�                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
	
	BSP_FLASH_WriteEnable();
	BSP_FLASH_WaitForWriteEnd();
	
	SPI_FLASH_CS_LOW();
  
	// ������������ָ��
	
	SPI_FLASH_SendByte(W25X_SectorErase);
	
	// ����Ҫ������������ַ
	
	SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(SectorAddr & 0xFF);
  
	SPI_FLASH_CS_HIGH();
 
	// �ȴ���������
	
	BSP_FLASH_WaitForWriteEnd();
	
	// �ͷŶ˿�
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_BufferWrite()
*
* Description : FLASH ����д��
*
* Argument(s) : pBuffer         :Ҫд��Ļ�����
*               WriteAddr       :Ҫд��ĵ�ַ
*               NumByteToWrite  :Ҫд����ֽ���
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
	
	// ����FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // ���õȴ�                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // �����ȴ�                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	

	Addr = WriteAddr % SPI_FLASH_PageSize;
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    if (Addr == 0) {                                                              // ������ַҳ�����
		if (NumOfPage == 0) {
			BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
		} else {                                                                  // ��ʾ������ҳ
		
			// д������ҳ
			
			while (NumOfPage--) {                                                 // д����ҳ
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}

			BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
		}
	} else {                                                                      // ������ַδҳ�����
		if (NumOfPage == 0) {                                                     // û������ҳ
			if (NumOfSingle > count) {
				temp = NumOfSingle - count;
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, count);
				WriteAddr +=  count;
				pBuffer += count;
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, temp);
			} else {
				BSP_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
			}
		} else {                                                                  // ������ҳ
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			BSP_FLASH_PageWrite(pBuffer, WriteAddr, count);
			WriteAddr +=  count;
			pBuffer += count;

			// д������ҳ
			
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
  
	// �ͷŶ˿�
	
	OSSemPost((OS_SEM *)& SemLock,
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err); 
}


/*
*********************************************************************************************************
*                                             BSP_FLASH_BufferRead()
*
* Description : FLASH ��������
*
* Argument(s) : pBuffer         :Ҫд��Ļ�����
*               WriteAddr       :Ҫ���ĵ�ַ
*               NumByteToWrite  :Ҫ�����ֽ���
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
	
	// ����FLASH
	
	OSSemPend((OS_SEM *)& SemLock,
	          (OS_TICK )  0,                                   // ���õȴ�                            
	          (OS_OPT  )  OS_OPT_PEND_BLOCKING,                // �����ȴ�                        
	          (CPU_TS *)  0,
	          (OS_ERR *)  &err);
	
    SPI_FLASH_CS_LOW();

	// ���Ͷ�ָ����ҷ��Ͷ���ַ
	
    SPI_FLASH_SendByte(W25X_ReadData);
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

	// ��ȡ������
	
	SPI_FLASH_ReadBuff(pBuffer, NumByteToRead);

	SPI_FLASH_CS_HIGH();
  
	// �ͷŶ˿�
	
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
* Description : Flashдʹ��
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
* Description : оƬæ�ȴ���������д�붼�Ǻ�ʱ�ģ��ú����ǵȴ�FlashоƬ����
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_FLASH_SectorErase(), BSP_FLASH_PageWrite()
*
* Note(s)     : �ú����������ʱ����
*********************************************************************************************************
*/


static void BSP_FLASH_WaitForWriteEnd(void)
{
	OS_ERR   err;
	uint8_t  FLASH_Status = 0;
	
	SPI_FLASH_CS_LOW();

	SPI_FLASH_SendByte(W25X_ReadStatusReg);                      // ��ȡ״̬������ָ��
	
	do {
		// ��ʱ1ms,��ΪFLASHҳ���ʱ�����ֵ��0.7ms
		
		OSTimeDly((OS_TICK )  1, 
				  (OS_OPT  )  OS_OPT_TIME_DLY, 
				  (OS_ERR *)& err);
		
		FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);           //��ȡ״̬�Ĵ���
		
	} while (SET == (FLASH_Status & WIP_Flag));
	
	SPI_FLASH_CS_HIGH();
}




/*
*********************************************************************************************************
*                                             BSP_FLASH_PageWrite()
*
* Description : FLASH ҳд�뺯��
*
* Argument(s) : pBuffer         :Ҫд��Ļ�����
*               WriteAddr       :Ҫд��ĵ�ַ
*               NumByteToWrite  :Ҫд����ֽ���
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
	
	// ����ҳ���ָ��
	
	SPI_FLASH_SendByte(W25X_PageProgram);
	  
	// ����ҳ��ַ
	
	SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(WriteAddr & 0xFF);

	// �ж�д��ʱ�򳬹�һҳ
	
	if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
	{
		NumByteToWrite = SPI_FLASH_PerWritePageSize;
	}
	
	// ������������д��
	
	SPI_FLASH_WriteBuff(pBuffer, NumByteToWrite);              
	
	SPI_FLASH_CS_HIGH();

	// �ȴ�Ҳд�����
	
	BSP_FLASH_WaitForWriteEnd();
}








