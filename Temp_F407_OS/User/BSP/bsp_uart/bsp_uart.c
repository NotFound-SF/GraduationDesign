/*
*********************************************************************************************************
*        该驱动函数使用了信号量，所以不能在ISR调用该文件函数，并且SemLock默认初始化为1所以读写互斥
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_uart.h"


/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct {
	OS_SEM     SemLock;                                       /* 该UART独占信号量，读写同时占用                    */
    OS_SEM     SemRdWait;                                     /* UART读等待信号量                                 */
	OS_SEM     SemTxWait;                                     /* UART发送等待信号量                               */  
	CPU_INT16U BufLen;                                        /* UART期望读取的数据长度                           */
	CPU_INT08U *BufPtr;                                       /* UART读缓冲区的头指针                             */
}BSP_UART_DEV_STATUS; 


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_UART_DEV_STATUS  BSP_UART_DevTbl[BSP_UART_NBR_MAX];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void        BSP_UARTx_Handler  (CPU_INT08U uart_id);

static  void        NVIC_Configuration(CPU_INT08U uart_id);
static  void        BSP_UART_GPIO_Init(CPU_INT08U uart_id);
static  void        BSP_UART_SEM_Creat(CPU_INT08U uart_id);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        BSP_UART_Init()
*
* Description : Initialize the UART.
*
* Argument(s) : uart_id    BSP_UART_ID_x  x = [1,6]
*
*               baud       波特率
*
* Return(s)   : NONE
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_UART_Init (CPU_INT08U uart_id, CPU_INT32U uart_baud)
{
	USART_TypeDef     *uartPort;                                             /* UART端口号                         */
	USART_InitTypeDef uartInit;
	
	BSP_UART_SEM_Creat(uart_id);                                             /* 创建相应的信号量                    */
	BSP_UART_GPIO_Init(uart_id);                                             /* 将GPIO链接到UART                   */
	NVIC_Configuration(uart_id);                                             /* 链接到NVIC                         */
	
	uartInit.USART_BaudRate            = uart_baud;                          /* 波特率                             */
	uartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;     /* 不使用硬件流控制                    */
	uartInit.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;      /* 可收发                             */ 
	uartInit.USART_StopBits            = USART_StopBits_1;                   /* 停止位1位                          */
	uartInit.USART_WordLength          = USART_WordLength_8b;                /* 数据宽度8位                        */
	uartInit.USART_Parity              = USART_Parity_No;                    /* 不使用校验                         */
	
	switch (uart_id) {
		case BSP_UART_ID_1:
			UART1_CLK_CMD(UART1_DEV_CLK, ENABLE);
			uartPort = USART1;
	    	break;
		
		case BSP_UART_ID_2:
			UART2_CLK_CMD(UART2_DEV_CLK, ENABLE);
			uartPort = USART2;
	    	break;
		
		case BSP_UART_ID_3:
			UART3_CLK_CMD(UART3_DEV_CLK, ENABLE);
			uartPort = USART3;
	    	break;
		
		case BSP_UART_ID_4:
			UART4_CLK_CMD(UART4_DEV_CLK, ENABLE);
			uartPort = UART4;
	    	break;
		
		case BSP_UART_ID_5:
			UART5_CLK_CMD(UART5_DEV_CLK, ENABLE);
			uartPort = UART5;
	    	break;
		
		case BSP_UART_ID_6:
			UART6_CLK_CMD(UART6_DEV_CLK, ENABLE);
			uartPort = USART6;
		
		default:
			break;
	}
	
	USART_ITConfig(uartPort, USART_IT_IDLE, DISABLE);                            /* 关闭空闲中断，即超时中断                  */
	USART_ITConfig(uartPort, USART_IT_RXNE, DISABLE);                            /* 关闭接收中断                             */
	USART_ITConfig(uartPort, USART_IT_TXE, DISABLE);                             /* 关闭发送中断                             */
	
	USART_Init(uartPort, &uartInit);
	USART_Cmd(uartPort, ENABLE); 
}


/*
*********************************************************************************************************
*                                        BSP_UART_RdByte()
*
* Description : 读取端口为uart_id的UATR数据，读取一个字节.
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*
*             : timeOut     读取最长的等待时间
*                                     
*
* Return(s)   : 返回读取到的数据
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

CPU_INT08U  BSP_UART_RdByte (CPU_INT08U uart_id, OS_TICK timeOut)
{
	CPU_INT08U data;
	
	BSP_UART_RdData(uart_id, &data, 1,timeOut);
	
	return data;
}


/*
*********************************************************************************************************
*                                        BSP_UART_WrByte()
*
* Description : 向端口为uart_id的UATR写一个字节数据
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                                     
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

void  BSP_UART_WrByte (CPU_INT08U uart_id, CPU_INT08U data)
{
	BSP_UART_WrData(uart_id, &data, 1);
}



/*
*********************************************************************************************************
*                                        BSP_UART_WrStr()
*
* Description : 向端口为uart_id的UATR发送一个字符串
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                                     
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

void  BSP_UART_WrStr (CPU_INT08U uart_id, CPU_CHAR *data)
{
	CPU_INT08U len = 0;
	
	while(0 != *(data+len)) {
		len++;
	}
	
	BSP_UART_WrData(uart_id, (CPU_INT08U*)data, len);
}



/*
*********************************************************************************************************
*                                        BSP_UART_RdData()
*
* Description : 读取端口为uart_id的UATR数据，读取长度为len.
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                             
*               data        指向接收缓冲区的头指针      
*                            
*               len         想要接收的数据长度    
*
*               timeOut     表示必须在该时间内读取完数据，当数据连续不断的发送并且接收正常时也会超时退出，所以调用者
*                           要确保在指定时间内能读取完所有数据；
*
* Return(s)   : 返回实际读取的数据长度
*
* Caller(s)   : Application
*
* Note(s)     : 该函数带超时检测，即读取到的实际数据长度可能比len小
*********************************************************************************************************
*/

static __IO uint8_t   ReadNoUse;                                               /* 用于在每次读取数据前读取相应的寄存器，以达到清除标志位的目的 */

CPU_INT16U  BSP_UART_RdData (CPU_INT08U uart_id, CPU_INT08U *data, CPU_INT16U len, OS_TICK timeOut)
{
	
	OS_ERR         err;
	USART_TypeDef *uartPort;
	CPU_INT16U     rdLen = 0;
	BSP_UART_DEV_STATUS *p_uart_dev_status;
	
	// 参数有效性检查
	
	if (uart_id >= BSP_UART_NBR_MAX)
		return 0;
	
	if (0 == len)                                          
		return 0;
	
	p_uart_dev_status = BSP_UART_DevTbl+uart_id;                               /* 链接到当前端口使用的设备状态结构体        */
																			   /* 先请求信号量防止结构体中数据被冲掉        */
	OSSemPend((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_TICK ) BSP_UAER_LOCK_TIME,                                                             
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 /* 阻塞等待                                */
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return 0;                    
	
	p_uart_dev_status->BufLen = len;                                           /* 准备读取的数据长度                       */
	p_uart_dev_status->BufPtr = data;                                          /* 读取到数据的存储缓冲区                   */
	                                                                         
	switch (uart_id) {                                                         /* 匹配端口号                              */
		case BSP_UART_ID_1:
			uartPort = USART1;
			break;
		
		case BSP_UART_ID_2:
			uartPort = USART2;
			break;
		
		case BSP_UART_ID_3:
			uartPort = USART3;
			break;
		
		case BSP_UART_ID_4:
			uartPort = UART4;
			break;
		
		case BSP_UART_ID_5:
			uartPort = UART5;
			break;
		
		case BSP_UART_ID_6:
			uartPort = USART6;
			break;
		
		default :
			break;
	}
	
         
	// 在读取数据前必须清除中断相应的标志，否则在未调用读函数时该总线上有数据流动
	// 那么及时当总线不再有数据流动式再调用读取函数时会立即返回，因为空闲中断标志
	// 置位，所以一开启中断就会导致函数立即返回，下面是清除空闲标志的方法
	                           
	ReadNoUse = uartPort -> SR;	                                              
	ReadNoUse = uartPort -> DR;                    
	
	// 开启空闲中断与接收中断
	
	uartPort -> CR1 |= UART_IDEL_RXN_MASK;            
	
	// 等待空闲中断返回
	OSSemPend((OS_SEM *) &(p_uart_dev_status->SemRdWait),                       /* 等待接收完成                            */
	          (OS_TICK ) timeOut,                                               /* 使函数不会永远阻塞                       */
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                  /* 阻塞等待                                */
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
			  
	// 关闭空闲中断与接收中断
	
	uartPort -> CR1 &= ~UART_IDEL_RXN_MASK; 

	// 计算读取到的数据长度
	
	rdLen = len-(p_uart_dev_status->BufLen);                                    /* 必须在释放端口前计算                     */
	
	// 如果是等待超时返回就需要确保SemRdWait信号量的值为0，因为当等待超时返回时并且在关闭中断之前
	// 一个数据到达导致空闲中断执行，那么中断就会导致SemRdWait加一，那么下次调用读数据函数时就会
	// 立即返回，这并不是我们期望的，虽然出现上面情况的概率相当小，但是我们还是应该确保在关闭中断后
	// 清零该信号量；
	
	if (OS_ERR_TIMEOUT == err && 0 != rdLen ) {                                /* 至少进入过中断才可能 */                                                                    
		OSSemSet((OS_SEM   *) &(p_uart_dev_status->SemRdWait),
				 (OS_SEM_CTR) 0,
		         (OS_ERR   *) &err);
	}
	
	
	                                                                            /* 释放该端口                              */
	OSSemPost((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &err);
	
	return rdLen;                                                              /* 返回读取到的实际字节数                   */
}


/*
*********************************************************************************************************
*                                        BSP_UART_WrData()
*
* Description : 读取端口为uart_id的UATR数据，读取长度为len.
*
* Argument(s) : uart_id     UART peripheral ID
*                           BSP_UART_ID_x   x=[1,5]
*                             
*               data        指向发送缓冲区的头指针      
*                            
*               len         将要发送的数据长度               
*
* Return(s)   : 实际写出的数据长度
*
* Caller(s)   : Application
*
* Note(s)     : none
*********************************************************************************************************
*/

CPU_INT16U  BSP_UART_WrData (CPU_INT08U uart_id, CPU_INT08U *data, CPU_INT16U len)
{
	OS_ERR  err;
	USART_TypeDef *uartPort;
	CPU_INT16U rdLen = 0;
	BSP_UART_DEV_STATUS *p_uart_dev_status;
	
	if(0 == len)
		return 0;
	if(uart_id >= BSP_UART_NBR_MAX)
		return 0;
	p_uart_dev_status = BSP_UART_DevTbl+uart_id;
	
	OSSemPend((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_TICK ) BSP_UAER_LOCK_TIME,                                   /* 如果永久等待可能导致任务永久挂起          */                           
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,                                 /* 阻塞等待                                */
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return 0;     
	
	switch (uart_id) {                                                         /* 匹配端口号                              */
		case BSP_UART_ID_1:
			uartPort = USART1;
			break;
		
		case BSP_UART_ID_2:
			uartPort = USART2;
			break;
		
		case BSP_UART_ID_3:
			uartPort = USART3;
			break;
		
		case BSP_UART_ID_4:
			uartPort = UART4;
			break;
		
		case BSP_UART_ID_5:
			uartPort = UART5;
			break;
		
		case BSP_UART_ID_6:
			uartPort = USART6;
			break;
		
		default :
			break;
		
	}
	
	USART_SendData(uartPort, *data);                                           /* 这样方可触发第一次发送中断                 */
	p_uart_dev_status->BufLen = len-1;                                         /* 准备写出的数据长度                        */
	p_uart_dev_status->BufPtr = data+1;                                        /* 数据的存储缓冲区                          */ 
	USART_ITConfig(uartPort, USART_IT_TXE, ENABLE);                            /* 开启发送完成中断中断                      */
	 
	OSSemPend((OS_SEM *)&(p_uart_dev_status->SemTxWait),                       /* 等待发送完成                              */
	          (OS_TICK )BSP_UAER_LOCK_TIME,
	          (OS_OPT  )OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
//	USART_ITConfig(uartPort, USART_IT_TXE, DISABLE);                           /* 在中断中关闭该中断                         */
	rdLen = len-(p_uart_dev_status->BufLen);                  
	
	OSSemPost((OS_SEM *)&(p_uart_dev_status->SemLock),
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &err);
	
	return rdLen;
}



/*
*********************************************************************************************************
*                                           BSP_UART_Printf()
*
* Description : Print formatted data to the output serial port.
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*               format      String that contains the text to be written.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function output a maximum of BSP_SER_PRINTF_STR_BUF_SIZE number of bytes to the
*                   serial port.  The calling function hence has to make sure the formatted string will
*                   be able fit into this string buffer or hence the output string will be truncated.
*********************************************************************************************************
*/
void  BSP_UART_Printf (CPU_INT08U uart_id, CPU_CHAR  *format, ...)
{
    CPU_CHAR  buf_str[BSP_SER_PRINTF_STR_BUF_SIZE + 1u];
    va_list   v_args;


    va_start(v_args, format);
   (void)vsnprintf((char       *)&buf_str[0],
                   (size_t      ) sizeof(buf_str),
                   (char const *) format,
                                  v_args);
    va_end(v_args);

    BSP_UART_WrStr(uart_id, buf_str);
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
*                                        NVIC_Configuration()
*
* Description : 初始化 UART 链接到NVIC
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UART_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  NVIC_Configuration(CPU_INT08U uart_id)
{
	NVIC_InitTypeDef nvicInit;
  
	switch (uart_id) {                                                           /* 配置中断源                      */
		case BSP_UART_ID_1:
			nvicInit.NVIC_IRQChannel = BSP_UART1_IRQ;
			break;
		
		case BSP_UART_ID_2:
			nvicInit.NVIC_IRQChannel = BSP_UART2_IRQ;
			break;
		
		case BSP_UART_ID_3:
			nvicInit.NVIC_IRQChannel = BSP_UART3_IRQ;
			break;
				
		case BSP_UART_ID_4:
			nvicInit.NVIC_IRQChannel = BSP_UART4_IRQ;
			break;
		
		case BSP_UART_ID_5:
			nvicInit.NVIC_IRQChannel = BSP_UART5_IRQ;
			break;
		
		case BSP_UART_ID_6:
			nvicInit.NVIC_IRQChannel = BSP_UART6_IRQ;
			break;
		
		default:
			break;
	}         

	nvicInit.NVIC_IRQChannelPreemptionPriority = BSP_UART_NVIC_PRE_PRIO;         /* 抢断优先级                      */
	nvicInit.NVIC_IRQChannelSubPriority        = BSP_UART_NVIC_SUB_PRIO;         /* 子优先级                        */
	nvicInit.NVIC_IRQChannelCmd                = ENABLE;                         /* 使能中断                        */
	
	NVIC_Init(&nvicInit);
}



/*
*********************************************************************************************************
*                                        BSP_UART_GPIO_Init()
*
* Description : 初始化 GPIO映射到UART
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UART_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_UART_GPIO_Init (CPU_INT08U uart_id)
{
	GPIO_InitTypeDef gpioInit_TX, gpioInit_RX;
	GPIO_TypeDef    *gpioPortTX, *gpioPortRX;
	
	switch (uart_id) {                                      
		case BSP_UART_ID_1:
			gpioInit_TX.GPIO_Pin = UART1_TX_PIN;                                /* 配置引脚                                 */
			gpioInit_RX.GPIO_Pin = UART1_RX_PIN;
			gpioPortTX = UART1_GPIO_PORT_TX;                                    /* 链接到端口                               */
			gpioPortRX = UART1_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART1_GPIO_CLK_TX, ENABLE);                  /* 开启GPIO端口时钟                         */
			RCC_AHB1PeriphClockCmd(UART1_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART1_TX_PIN_SOURCE, GPIO_AF_USART1);  /* GPIO功能复用                             */
			GPIO_PinAFConfig(gpioPortRX, UART1_RX_PIN_SOURCE, GPIO_AF_USART1);
			break;
		
		case BSP_UART_ID_2:
			gpioInit_TX.GPIO_Pin = UART2_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART2_RX_PIN;
		    gpioPortTX = UART2_GPIO_PORT_TX;
			gpioPortRX = UART2_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART2_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART2_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART2_TX_PIN_SOURCE, GPIO_AF_USART2);
			GPIO_PinAFConfig(gpioPortRX, UART2_RX_PIN_SOURCE, GPIO_AF_USART2);
			break;
		
		case BSP_UART_ID_3:
			gpioInit_TX.GPIO_Pin = UART3_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART3_RX_PIN;
			gpioPortTX = UART3_GPIO_PORT_TX;
			gpioPortRX = UART3_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART3_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART3_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART3_TX_PIN_SOURCE, GPIO_AF_USART3);
			GPIO_PinAFConfig(gpioPortRX, UART3_RX_PIN_SOURCE, GPIO_AF_USART3);
			break;
				
		case BSP_UART_ID_4:
			gpioInit_TX.GPIO_Pin = UART4_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART4_RX_PIN;
			gpioPortTX = UART4_GPIO_PORT_TX;
			gpioPortRX = UART4_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART4_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART4_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART4_TX_PIN_SOURCE, GPIO_AF_UART4);
			GPIO_PinAFConfig(gpioPortRX, UART4_RX_PIN_SOURCE, GPIO_AF_UART4);
			break;
		
		case BSP_UART_ID_5:
			gpioInit_TX.GPIO_Pin = UART5_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART5_RX_PIN;
			gpioPortTX = UART5_GPIO_PORT_TX;
			gpioPortRX = UART5_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART5_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART5_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART5_TX_PIN_SOURCE, GPIO_AF_UART5);
			GPIO_PinAFConfig(gpioPortRX, UART5_RX_PIN_SOURCE, GPIO_AF_UART5);
			break;
		
		case BSP_UART_ID_6:
			gpioInit_TX.GPIO_Pin = UART6_TX_PIN;
			gpioInit_RX.GPIO_Pin = UART6_RX_PIN;
			gpioPortTX = UART6_GPIO_PORT_TX;
			gpioPortRX = UART6_GPIO_PORT_RX;
			RCC_AHB1PeriphClockCmd(UART6_GPIO_CLK_TX, ENABLE);
			RCC_AHB1PeriphClockCmd(UART6_GPIO_CLK_RX, ENABLE);
			GPIO_PinAFConfig(gpioPortTX, UART6_TX_PIN_SOURCE, GPIO_AF_USART6);
			GPIO_PinAFConfig(gpioPortRX, UART6_RX_PIN_SOURCE, GPIO_AF_USART6);
			break;
		
		default:
			break;
	}    
	
	gpioInit_TX.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_TX.GPIO_OType = GPIO_OType_PP;
	gpioInit_TX.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit_TX.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortTX, &gpioInit_TX);
	
	gpioInit_RX.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_RX.GPIO_OType = GPIO_OType_OD;                    
	gpioInit_RX.GPIO_PuPd  = GPIO_PuPd_UP;
	gpioInit_RX.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortRX, &gpioInit_RX);
}


/*
*********************************************************************************************************
*                                        BSP_UART_SEM_Creat()
*
* Description : 创建UART所需信号量
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UART_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_UART_SEM_Creat (CPU_INT08U uart_id)
{
	OS_ERR err;
	
	if(uart_id >= BSP_UART_NBR_MAX)
		return;
	
	OSSemCreate((OS_SEM    *)&((BSP_UART_DevTbl+uart_id)->SemLock),
                (CPU_CHAR  *)"UART Lock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	OSSemCreate((OS_SEM    *)&((BSP_UART_DevTbl+uart_id)->SemTxWait),
                (CPU_CHAR  *)"UART Tx Wait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
	
	OSSemCreate((OS_SEM    *)&((BSP_UART_DevTbl+uart_id)->SemRdWait),
                (CPU_CHAR  *)"UART Rd Wait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
}



/*
*********************************************************************************************************
*                                        BSP_UARTx_Handler()
*
* Description : 中断处理函数
*
* Argument(s) : uart_id     BSP_UART_ID_x   x=[1,6]
*
* Return(s)   : none
*
* Caller(s)   : BSP_UARTx_Handler
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_UARTx_Handler (CPU_INT08U uart_id)
{
	OS_ERR err;
	USART_TypeDef *uartPort;
	BSP_UART_DEV_STATUS *p_uart_dev_status;
	
	p_uart_dev_status = BSP_UART_DevTbl+uart_id;                               /* 链接到当前端口使用的设备状态结构体        */
	
	switch (uart_id) {                                                         /* 匹配端口号                              */
		case BSP_UART_ID_1:
			uartPort = USART1;
			break;
		
		case BSP_UART_ID_2:
			uartPort = USART2;
			break;
		
		case BSP_UART_ID_3:
			uartPort = USART3;
			break;
		
		case BSP_UART_ID_4:
			uartPort = UART4;
			break;
		
		case BSP_UART_ID_5:
			uartPort = UART5;
			break;
		
		case BSP_UART_ID_6:
			uartPort = USART6;
			break;
		
		default :
			break;
	}
	
	if (SET == USART_GetITStatus(uartPort, USART_IT_RXNE)) {                  /* 接收缓冲区非空                            */
		if(0 != p_uart_dev_status->BufLen) {                                  /* 表示接收未完成                            */
			*(p_uart_dev_status->BufPtr) = USART_ReceiveData(uartPort);       /* 填写接收缓冲区                            */
			p_uart_dev_status->BufPtr++;
			p_uart_dev_status->BufLen--;                                         
		} 
		
		USART_ClearITPendingBit(uartPort, USART_IT_RXNE);                     /* 清零标志位                                */                                  
	}
	
	if (SET == USART_GetITStatus(uartPort, USART_IT_IDLE)) {                  /* 表示一帧数据发送结束                      */
		USART_ReceiveData(uartPort);                                          /* 清零标志位                               */ 
		OSSemPost((OS_SEM *)&(p_uart_dev_status->SemRdWait),
	              (OS_OPT  )OS_OPT_POST_1,
	              (OS_ERR *) &err);
	}	

	if (SET == USART_GetITStatus(uartPort, USART_IT_TXE)) {                  /* 发送完一个字节                           */     
		if(0 != p_uart_dev_status->BufLen) {                                 /* 当发送缓冲区数据还未发送结束               */                           
			USART_SendData(uartPort, *(p_uart_dev_status->BufPtr));
			p_uart_dev_status->BufPtr++;
			p_uart_dev_status->BufLen--;   
		} else {                                                             /* 表示发送结束                              */
			USART_ITConfig(uartPort, USART_IT_TXE, DISABLE);                 /* 关闭中断                                  */
			OSSemPost((OS_SEM *)&(p_uart_dev_status->SemTxWait),
	              (OS_OPT  )OS_OPT_POST_1,
	              (OS_ERR *) &err);
		}       
	}
	
}


/*
*********************************************************************************************************
*                                        BSP_UART1_Handler()
*
* Description : UART1中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 系统
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART1_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_1);
}


/*
*********************************************************************************************************
*                                        BSP_UART2_Handler()
*
* Description : UART2中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 系统
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART2_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_2);
}


/*
*********************************************************************************************************
*                                        BSP_UART3_Handler()
*
* Description : UART3中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 系统
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART3_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_3);
}


/*
*********************************************************************************************************
*                                        BSP_UART4_Handler()
*
* Description : UART4中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 系统
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART4_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_4);
}


/*
*********************************************************************************************************
*                                        BSP_UART5_Handler()
*
* Description : UART5中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 系统
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART5_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_5);
}


/*
*********************************************************************************************************
*                                        BSP_UART5_Handler()
*
* Description : UART5中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 系统
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_UART6_Handler (void)
{
	BSP_UARTx_Handler(BSP_UART_ID_6);
}




