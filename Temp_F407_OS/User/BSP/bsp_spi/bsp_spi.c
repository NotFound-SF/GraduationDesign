/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_spi.h"


/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/


// 读写操作选项标志,读写同时进行，只读，只写

typedef enum {
	SPIxReadWrite = 0x00,
	SPIxReadOnly  = 0x01,
	SPIxWriteOnly = 0x02
}Read_Write_Flag;


// SPI状态全局变量

typedef struct {
	OS_SEM          SemLock;                                       // 该SPI端口独占信号量                  
    OS_SEM          SemWait;                                       // SPI读等待信号量                                 
	CPU_INT16U      RdBufLen;                                      // SPI需要读取的数据长度                           
	CPU_INT16U      WrBufLen;                                      // SPI需要写入的数据长度                           
	CPU_INT08U      *RdBufPtr;                                     // SPI读缓冲区的头指针                             
	CPU_INT08U      *WrBufPtr;                                     // SPI读写缓冲区的头指针                             
	Read_Write_Flag Operation;                                     // 读写操作标志flag     
}BSP_SPIx_DEV_STATUS; 


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_SPIx_DEV_STATUS         BSP_SPIx_DevStatus;                // 用于表征SPI工作状态，也是与中断沟通的枢纽

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static      void         BSP_NVIC_Configuration(void);
static      void         BSP_GPIO_Init(void);

/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_SPI_Init()
*
* Description : 初始化 SPI引脚，映射到SPI外设，配置PSI到相应的模式
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_SPIx_Init(void)
{
	OS_ERR err;
	
	SPI_InitTypeDef  SPI_InitStructure;
	
	// 开启SPI时钟
	
	BSP_SPIx_RCC_CMD(BSP_SPIx_RCC, ENABLE);
	
	// 配置SPI相关的引脚
	
	BSP_GPIO_Init();
	
	// 将SPI连接到中断
	
	BSP_NVIC_Configuration();
	
	// 创建相应的信号量
	
	OSSemCreate((OS_SEM    *)& (BSP_SPIx_DevStatus.SemLock),            // 端口锁定信号量
                (CPU_CHAR  *)  "SPI Lock", 
	            (OS_SEM_CTR )  1,
	            (OS_ERR    *)& err);
				
	OSSemCreate((OS_SEM    *)& (BSP_SPIx_DevStatus.SemWait),            // 端口等待信号量
                (CPU_CHAR  *)  "SPI Wait", 
	            (OS_SEM_CTR )  0,
	            (OS_ERR    *)& err);
	
	
	// 配置SPI模式
	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                       // 工作主机模式
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                         // 时钟极性
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                        // 时钟相位
	SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;                          // 软件作为片选
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                  // 高位先行
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                   // 数据宽度8bit
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  // 工作在全双工模式
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  // 时钟分屏数
	SPI_InitStructure.SPI_CRCPolynomial = 7;                            // CRC校验
	
	SPI_Init(BSP_SPIx_PORT, &SPI_InitStructure);                       
	BSP_SPIx_CS_DISABLE();                                              // 确保SPI片选不使能
	SPI_Cmd(BSP_SPIx_PORT, ENABLE);                                     // 使能SPI	
}



/*
*********************************************************************************************************
*                                        BSP_SPI_ReadWrite()
*
* Description : 向SPI总线发送BuffLen个字节，并且读取BuffLen个字节到ReadBuff缓冲区
*
* Argument(s) : WrBuff   ：发送缓冲区
*               ReadBuff ：接收缓冲区
*               BuffLen  : 发送与接收缓冲长度       
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 该函数是全双工函数，读写同时进行.
*********************************************************************************************************
*/

void BSP_SPI_ReadWrite(uint8_t *WrBuff, uint8_t *ReadBuff, uint16_t BuffLen) 
{
	OS_ERR err;
	
	// 参数检测
	
	if (0 == BuffLen) 
		return;
	
	// 锁定SPI端口
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemLock),
	          (OS_TICK ) 0,                                                    // 永久等待                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // 阻塞等待                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 初始化妆台变量
	
	BSP_SPIx_DevStatus.RdBufLen = BuffLen;                                  
	BSP_SPIx_DevStatus.RdBufPtr = ReadBuff;
	BSP_SPIx_DevStatus.WrBufPtr = WrBuff;                                   
	BSP_SPIx_DevStatus.WrBufLen = BuffLen;
	BSP_SPIx_DevStatus.Operation = SPIxReadWrite;
	
	// 开启读写中断开启顺序很关键，因为空闲时TXE是置位的，而RXDN是0的

	BSP_SPIx_PORT->CR2 |= DEF_BIT_06;                                         // 开启接收中断
	BSP_SPIx_PORT->CR2 |= DEF_BIT_07;                                         // 开启发送中断会自动进入中断开始发送第一个数据
	
	// 等待读写结束
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemWait),
	          (OS_TICK ) 0,                                                    // 永久等待                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // 阻塞等待                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 关闭读写中断
	
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_06;                                         // 关闭接收中断
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                         // 关闭发送中断    
	
	
	// 释放端口
	
	OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemLock),
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}



/*
*********************************************************************************************************
*                                        BSP_SPI_ReadOnly()
*
* Description : 读取BuffLen个字节到ReadBuff缓冲区，并且向SPI总线发送BuffLen个无效字节
*
* Argument(s) : ReadBuff ：接收缓冲区
*               BuffLen  : 发送与接收缓冲长度       
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 该函数是全双工函数，读写同时进行.
*********************************************************************************************************
*/

void BSP_SPI_ReadOnly(uint8_t *ReadBuff, uint16_t BuffLen) 
{
	OS_ERR err;
	
	// 参数检测
	
	if (0 == BuffLen) 
		return;
	
	// 锁定端口
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemLock),
	          (OS_TICK ) 0,                                                    // 永久等待                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // 阻塞等待                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 初始化妆台变量
	
	BSP_SPIx_DevStatus.RdBufLen = BuffLen;                                    //无论是只读只写都要将两个BufLen赋相同的长度
	BSP_SPIx_DevStatus.WrBufLen = BuffLen;
	BSP_SPIx_DevStatus.RdBufPtr = ReadBuff;
	BSP_SPIx_DevStatus.Operation = SPIxReadOnly;
	
	// 开启读写中断
	
	BSP_SPIx_PORT->CR2 |= DEF_BIT_06;                                         // 开启接收中断
	BSP_SPIx_PORT->CR2 |= DEF_BIT_07;                                         // 开启发送中断会自动进入中断开始发送第一个数据
	
	// 等待读取结束
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemWait),
	          (OS_TICK ) 0,                                                    // 永久等待                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // 阻塞等待                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 关闭读写中断
	
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_06;                                         // 关闭接收中断
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                         // 关闭发送中断  
	
	// 释放端口
	
	OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemLock),
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}



/*
*********************************************************************************************************
*                                        BSP_SPI_WriteOnly()
*
* Description : 向SPI总线发送BuffLen个字节,并且忽略接收到的数据
*
* Argument(s) : WriteBuff：接收缓冲区
*               BuffLen  : 发送与接收缓冲长度       
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : 该函数是全双工函数，读写同时进行.
*********************************************************************************************************
*/

void BSP_SPI_WriteOnly(uint8_t *WriteBuff, uint16_t BuffLen) 
{
	OS_ERR err;
	
	// 参数检测
	
	if (0 == BuffLen) 
		return;
	
	// 锁定端口
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemLock),
	          (OS_TICK ) 0,                                                    // 永久等待                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // 阻塞等待                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 初始化妆台变量
	
	BSP_SPIx_DevStatus.RdBufLen = BuffLen;                                    //无论是只读只写都要将两个BufLen赋相同的长度
	BSP_SPIx_DevStatus.WrBufLen = BuffLen;
	BSP_SPIx_DevStatus.WrBufPtr = WriteBuff;
	BSP_SPIx_DevStatus.Operation = SPIxWriteOnly;
	
	// 开启读写中断
	
	BSP_SPIx_PORT->CR2 |= DEF_BIT_06;                                         // 开启接收中断
	BSP_SPIx_PORT->CR2 |= DEF_BIT_07;                                         // 开启发送中断会自动进入中断开始发送第一个数据
	
	// 等待读取结束
	
	OSSemPend((OS_SEM *)&(BSP_SPIx_DevStatus.SemWait),
	          (OS_TICK ) 0,                                                    // 如果永久等待                            
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 // 阻塞等待                        
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 关闭读写中断
	
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_06;                                         // 关闭接收中断
	BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                         // 关闭发送中断  
	
	// 释放端口
	
	OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemLock),
	          (OS_OPT  )  OS_OPT_POST_1,
	          (OS_ERR *)  &err);
}


/*
*********************************************************************************************************
*                                        BSP_SPI_WriteByte()
*
* Description : 写一个字节
*
* Argument(s) : dat 要写入的数据      
*
* Return(s)   : 读取到的数据
*
* Caller(s)   : Application
*
* Note(s)     : none 
*********************************************************************************************************
*/

uint8_t BSP_SPI_WriteByte(uint8_t dat)
{
	uint8_t readDat;
	
	BSP_SPI_ReadWrite(&dat, &readDat, 1);
	
	return readDat;
}




/*
*********************************************************************************************************
*                                        BSP_SPI_ReadByte()
*
* Description : 读取一个字节
*
* Argument(s) : none      
*
* Return(s)   : 读取到的数据
*
* Caller(s)   : Application
*
* Note(s)     : none 
*********************************************************************************************************
*/

uint8_t BSP_SPI_ReadByte(void)
{	
	uint8_t res;
	
	BSP_SPI_ReadOnly(&res, 1);
	
	return res;
}



/*
*********************************************************************************************************
*                                        BSP_SPIx_IRQHandler()
*
* Description : SPI中断处理函数
*
* Argument(s) : none      
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

#define  BSP_SPIx_RXNE_MASK       0x01                  		 // 接收缓冲区非空标志位掩码
#define  BSP_SPIx_TXE_MASK        0x02                           // 发送缓冲区空标志位掩码

// 定义为全局为了提高效率，不用每次分配空间，供中断使用
static __IO uint16_t SPIx_CR2_Temp;                              // 暂存CR2中的数据
static __IO uint16_t SPIx_SR_Temp;                               // 在中断中暂存状态寄存器
static __IO uint16_t SPIx_DR_NULL;                               // 当不需要接收数据时存储接收寄存器值达到清标志位目的
static      OS_ERR   err;                                        // 信号量错误标志

void BSP_SPIx_IRQHandler(void)
{
	// 读取状态寄存器与中断寄存器
	
	SPIx_CR2_Temp = BSP_SPIx_PORT->CR2;
	SPIx_SR_Temp  = BSP_SPIx_PORT->SR;
	
	// 表示发送缓冲区为空
	
	if ((SPIx_SR_Temp & BSP_SPIx_TXE_MASK) && (SPIx_CR2_Temp & DEF_BIT_07)) {
		
		switch (BSP_SPIx_DevStatus.Operation) {
			
			// 当为只写或者为读写时需要将缓冲区填充到SPI数据寄存器
			
			case SPIxReadWrite:
			case SPIxWriteOnly:
				if (BSP_SPIx_DevStatus.WrBufLen> 0) {                                   
					BSP_SPIx_PORT -> DR = *(BSP_SPIx_DevStatus.WrBufPtr);               // 将缓冲区填充到SPI数据寄存器
					BSP_SPIx_DevStatus.WrBufPtr++;                                      // 缓冲指针加一       
					BSP_SPIx_DevStatus.WrBufLen--;                                      // 缓冲长度减一	
				}					
				break;
				
			// 当为只读时，需要向缓冲区填入垃圾值达到清标志位目的
				
			case SPIxReadOnly:
				if (BSP_SPIx_DevStatus.WrBufLen> 0) {         
					BSP_SPIx_PORT -> DR = BSP_SPI_NOUSE;                                // 只读时只需要填入垃圾值
					BSP_SPIx_DevStatus.WrBufLen--;                                      // 缓冲长度减一
				}
				break;
		}		
		
		if (0 == BSP_SPIx_DevStatus.WrBufLen) {                                                                      
			BSP_SPIx_PORT->CR2 &= ~DEF_BIT_07;                                          // 关发送中断
		}		
	}
	
	// 表示接收缓冲区非空
	
	if ((SPIx_SR_Temp & BSP_SPIx_RXNE_MASK) && (SPIx_CR2_Temp & DEF_BIT_06) ) {                                        
		
		switch (BSP_SPIx_DevStatus.Operation) {
			
			// 当为只写或者读写同时进行时
			
			case SPIxReadWrite:
			case SPIxReadOnly:
				*BSP_SPIx_DevStatus.RdBufPtr = BSP_SPIx_PORT -> DR;                   // 将数据写入缓存区
				BSP_SPIx_DevStatus.RdBufPtr++;
				BSP_SPIx_DevStatus.RdBufLen--;
				break;
				
			case SPIxWriteOnly:
				SPIx_DR_NULL = BSP_SPIx_PORT -> DR;                                   // 空读以清除该标准
				BSP_SPIx_DevStatus.RdBufLen--;                                        
				break;
		}		
		
		// SPI通讯结束点
		
		if (0 == BSP_SPIx_DevStatus.RdBufLen) {                               
			OSSemPost((OS_SEM *)& (BSP_SPIx_DevStatus.SemWait),
					  (OS_OPT  )  OS_OPT_POST_1,
					  (OS_ERR *)  &err);
		}
		
	}
	
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
*                                        BSP_NVIC_Configuration()
*
* Description : 初始化 SPI 链接到NVIC
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_SPIx_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_NVIC_Configuration(void)
{
	NVIC_InitTypeDef nvicInit;
	
	nvicInit.NVIC_IRQChannel                   = BSP_SPIx_IRQ;
	nvicInit.NVIC_IRQChannelPreemptionPriority = BSP_SPIx_NVIC_PRE_PRIO;         /* 抢断优先级                      */
	nvicInit.NVIC_IRQChannelSubPriority        = BSP_SPIx_NVIC_SUB_PRIO;         /* 子优先级                        */
	nvicInit.NVIC_IRQChannelCmd                = ENABLE;                         /* 使能中断                        */
	
	NVIC_Init(&nvicInit);
}




/*
*********************************************************************************************************
*                                        BSP_GPIO_Init()
*
* Description : 初始化 SPI引脚，映射到SPI外设
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_SPI_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
    // 开启GPIO时钟 
	
	RCC_AHB1PeriphClockCmd(BSP_SPIx_CS_GPIO_CLK, ENABLE);            // 开启片选引脚时钟
	RCC_AHB1PeriphClockCmd(BSP_SPIx_CLK_GPIO_CLK, ENABLE);           // 开启SCL引脚时钟
	RCC_AHB1PeriphClockCmd(BSP_SPIx_MOSI_GPIO_CLK, ENABLE);          // 开启MOSI引脚时钟
	RCC_AHB1PeriphClockCmd(BSP_SPIx_MISO_GPIO_CLK, ENABLE);          // 开启MISO引脚时钟
	
	// 引脚复用配置
	
	GPIO_PinAFConfig(BSP_SPIx_CLK_PORT, BSP_SPIx_CLK_SOURCE, BSP_SPIx_GPIO_AF);
	GPIO_PinAFConfig(BSP_SPIx_MISO_PORT, BSP_SPIx_MISO_SOURCE, BSP_SPIx_GPIO_AF);
	GPIO_PinAFConfig(BSP_SPIx_MOSI_PORT, BSP_SPIx_MOSI_SOURCE, BSP_SPIx_GPIO_AF);

	// 配置引脚
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_CLK_PIN;                 // 配置CLk引脚
	GPIO_Init(BSP_SPIx_CLK_PORT, &GPIO_InitStructure);
	 
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_MISO_PIN;                // 配置MISO引脚
	GPIO_Init(BSP_SPIx_MISO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_MOSI_PIN;                // 配置MOSI引脚
	GPIO_Init(BSP_SPIx_MOSI_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;                  // 配置CS引脚
	GPIO_InitStructure.GPIO_Pin = BSP_SPIx_CS_PIN;
	GPIO_Init(BSP_SPIx_CS_PORT, &GPIO_InitStructure);
}


