

/*
*********************************************************************************************************
*        该驱动函数使用了信号量，所以不能在ISR调用该文件函数，并且SemLock默认初始化为1所以读写互斥
*        同样该驱动函数是使用的主模式，即使从机发送起始信号与主机设备地址匹配，即会进入中断，但是默
*        认State被初始化为BSP_I2C_STATE_IDLE，所以在中断中并不会做进一步响应
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_iic.h"

/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct {
	OS_SEM                 SemLock;                                  //该IIC独占信号量                                
    OS_SEM                 SemWait;                                  //IIC等待信号量                                  
	CPU_INT16U             BufLen;                                   //期望读写的数据长度                              
	CPU_INT08U            *BufPtr;                                   //读写数据的缓冲区指针                            
	CPU_INT08U             Addr;                                     //从机地址   
	I2C_State_TypeDef      State;                                    //当前I2C状态  
	I2C_AccessType_TypeDef AccessType;                               //访问类型	                                   
}BSP_I2C_PERIPH_STATUS; 


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_I2C_PERIPH_STATUS  BSP_I2C_PeriphTbl[BSP_I2C_NBR_MAX]; 


/*
*********************************************************************************************************
*                                              LOCAL DEFINES
*********************************************************************************************************
*/

#define        ADDR_NONE                        0x00

#define        BSP_I2C_REG_SR1_ERR_MASK        (BSP_I2C_REG_SR1_BERR    | \
                                                BSP_I2C_REG_SR1_ARLO    | \
                                                BSP_I2C_REG_SR1_AF      | \
                                                BSP_I2C_REG_SR1_OVR     | \
                                                BSP_I2C_REG_SR1_PECERR  | \
												BSP_I2C_REG_SR1_TIMEOUT | \
                                                BSP_I2C_REG_SR1_SMBALERT)
                                                

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void         BSP_I2Cx_EV_Handler  (CPU_INT08U i2c_id);
static  void         BSP_I2Cx_ER_Handler  (CPU_INT08U i2c_id);

static  void         NVIC_Configuration   (CPU_INT08U i2c_id);
static  void         BSP_I2C_GPIO_Init    (CPU_INT08U i2c_id);
static  CPU_BOOLEAN  BSP_I2C_SEM_Creat    (CPU_INT08U i2c_id);

static  CPU_BOOLEAN  BSP_I2C_StartXfer ( CPU_INT08U              i2c_id,
                                         CPU_INT08U              i2c_addr,
                                         I2C_AccessType_TypeDef  i2c_access_type,
                                         CPU_INT08U             *p_buf,                    
                                         CPU_INT16U              nbr_bytes);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_I2C_Init()
*
* Description : Initialize the I2C.
*
* Argument(s) : i2c_id    BSP_I2C_ID_x                   x = [1,3]
*               i2c_mode  BSP_I2C_MODE_x                 x = STANDARD,FAST_2_1,FAST_16_9
*            	clk_freq  <= BSP_I2C_MODE_x_MAX_FREQ_HZ  x = STANDARD,FAST
*
* Return(s)   : DEF_OK    正常初始化
*               DEF_FAIL  初始化失败，可能是信号量
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Init (CPU_INT08U       i2c_id,
                           I2C_Mode_TypeDef i2c_mode,
						   CPU_INT32U       clk_freq)
						   
{
	CPU_BOOLEAN     status;
	I2C_TypeDef    *i2cPort;
	I2C_InitTypeDef i2cInit;
	
	BSP_I2C_GPIO_Init(i2c_id);                                           //初始化GPIO
	NVIC_Configuration(i2c_id);                                          //链接到中断
	status = BSP_I2C_SEM_Creat(i2c_id);                                  //创建对应的信号量
	
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return DEF_FAIL;
	
	if(clk_freq > BSP_I2C_MODE_FAST_MAX_FREQ_HZ) {                       //超过快速模式最高频率
		return DEF_FAIL;
	}
		
	if(BSP_I2C_MODE_STANDARD == i2c_mode) {
		if(clk_freq > BSP_I2C_MODE_STANDARD_MAX_FREQ_HZ)                 //超过标准模式模式最高频率
			return DEF_FAIL;
	}
	
	switch (i2c_id) {
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			I2C1_PERIPH_RCC_CMD(I2C1_PERIPH_RCC, ENABLE);                //开启外设时钟
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			I2C2_PERIPH_RCC_CMD(I2C2_PERIPH_RCC, ENABLE);                //开启外设时钟
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			I2C3_PERIPH_RCC_CMD(I2C3_PERIPH_RCC, ENABLE);                //开启外设时钟
			break;
		
		default:
			break;
	}
	      
	
	switch (i2c_mode) {                                                  //快速模式下时钟占空比
		case BSP_I2C_MODE_STANDARD:
		case BSP_I2C_MODE_FAST_2:
			i2cInit.I2C_DutyCycle   = I2C_DutyCycle_2; 
			break;
		
		case BSP_I2C_MODE_FAST_16_9:
			i2cInit.I2C_DutyCycle   = I2C_DutyCycle_16_9;
			break;
		
		default:
			break;
	}
	
	
	i2cInit.I2C_Mode        = I2C_Mode_I2C;                              //配置为I2C模式
	i2cInit.I2C_ClockSpeed  = clk_freq;                                  //时钟频率
	i2cInit.I2C_Ack         = I2C_Ack_Disable;                           //失能应答
	i2cInit.I2C_OwnAddress1 = I2C_OWN_ADDRESS7;                          //自身地址	                      
	i2cInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;      //7位地址模式   	

											                             //将端口设置为默认状态
	(BSP_I2C_PeriphTbl+(i2c_id)) -> BufLen = 0;                             
	(BSP_I2C_PeriphTbl+(i2c_id)) -> Addr   = ADDR_NONE;                    
	(BSP_I2C_PeriphTbl+(i2c_id)) -> BufPtr = (CPU_INT08U*)0;         
	(BSP_I2C_PeriphTbl+(i2c_id)) -> State  = BSP_I2C_STATE_IDLE;            
	(BSP_I2C_PeriphTbl+(i2c_id)) -> AccessType = BSP_I2C_ACCESS_TYPE_NONE; 
	
	I2C_Init(i2cPort, &i2cInit);
	I2C_Cmd(i2cPort, ENABLE);
	
	return status;
}



/*
*********************************************************************************************************
*                                        BSP_I2C_Rd()
*
* Description : Read 'n' bytes from the I2C bus.
*
* Argument(s) : i2c_id       BSP_I2C_ID_x              x = [1,3]
*
*               i2c_addr     The I2C device address
*
*               p_buf        Pointer to the buffer into which the bytes will be stored.
*
*               nbr_bytes    Number of bytes to be read.
*
* Return(s)   : DEF_OK       If all bytes were read.
*               DEF_FAIL     If all bytes could not be read.
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Rd (CPU_INT08U      i2c_id,
                            CPU_INT08U   i2c_addr,
                            CPU_INT08U  *p_buf,
                            CPU_INT16U   nbr_bytes)
{
	CPU_BOOLEAN  err;
	
	err = BSP_I2C_StartXfer(i2c_id,                                       //该函数带有参数检查
							i2c_addr,
							BSP_I2C_ACCESS_TYPE_RD, 
							p_buf, nbr_bytes);    

	return err;
}


/*
*********************************************************************************************************
*										  BSP_I2C_Wr()
*
* Description :  Write 'n' bytes from the I2C bus.
*
* Argument(s) :  i2c_id 	      BSP_I2C_ID_x				x = [1,3]
*
*				 i2c_addr	  The I2C device address
*
*				 p_buf		  Pointer to the buffer into which the bytes will be stored.
*
*				 nbr_bytes	  Number of bytes to be write.
*
* Return(s)   :  DEF_OK 	  If all bytes were write.
*				 DEF_FAIL	  If all bytes could not be write.
*
* Caller(s)   :  Application
*
* Note(s)	  :  none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_Wr (CPU_INT08U      i2c_id,
                            CPU_INT08U   i2c_addr,
                            CPU_INT08U  *p_buf,
                            CPU_INT16U   nbr_bytes)
{
 	CPU_BOOLEAN  err;
 
 	err = BSP_I2C_StartXfer(i2c_id,									   //该函数带有参数检查
						    i2c_addr,
						    BSP_I2C_ACCESS_TYPE_WR, 
						    p_buf, nbr_bytes);    

    return err;
}




/*
*********************************************************************************************************
*                                         BSP_I2C_WrRd()
*
* Description :  先向总线写一个字节缓冲区首字节，然后重新发送起始信号从总线上读取n个字节，并且缓冲区首字节被读入数据覆盖.
*
* Argument(s) :  i2c_id       BSP_I2C_ID_x              x = [1,3]
*
*                i2c_addr     The I2C device address
*
*                p_buf        Pointer to the buffer into which the bytes will be stored.
*
*                nbr_bytes    Number of bytes to be read.
*
* Return(s)   :  DEF_OK       If all bytes were read.
*                DEF_FAIL     If all bytes could not be read.
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_I2C_WrRd (CPU_INT08U       i2c_id,
                               CPU_INT08U   i2c_addr,
                               CPU_INT08U  *p_buf,
                               CPU_INT16U   nbr_bytes)
{
    CPU_BOOLEAN  err;
                             
    err = BSP_I2C_StartXfer(i2c_id,                                    //该函数带有参数检查
                            i2c_addr,
                            BSP_I2C_ACCESS_TYPE_WR_RD, 
                            p_buf, nbr_bytes); 

    return err;
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
*                                        BSP_I2C_GPIO_Init()
*
* Description : 初始化 GPIO映射到I2C外设
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2C_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_I2C_GPIO_Init (CPU_INT08U i2c_id)
{
	GPIO_InitTypeDef gpioInit_SCL, gpioInit_SDA;
	GPIO_TypeDef *gpioPortSCL, *gpioPortSDA;
	
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return;
	
	switch (i2c_id) {
		case BSP_I2C_ID_1:
			gpioPortSDA = I2C1_GPIO_PORT_SDA;
		    gpioPortSCL = I2C1_GPIO_PORT_SCL;
			gpioInit_SDA.GPIO_Pin = I2C1_SDA_PIN;
			gpioInit_SCL.GPIO_Pin = I2C1_SCL_PIN;
			RCC_AHB1PeriphClockCmd(I2C1_GPIO_SDA_RCC, ENABLE);
			RCC_AHB1PeriphClockCmd(I2C1_GPIO_SCL_RCC, ENABLE);
			GPIO_PinAFConfig(gpioPortSDA, I2C1_SDA_PIN_SOURCE, GPIO_AF_I2C1);
			GPIO_PinAFConfig(gpioPortSCL, I2C1_SCL_PIN_SOURCE, GPIO_AF_I2C1);
			break;
		
		case BSP_I2C_ID_2:
			gpioPortSDA = I2C2_GPIO_PORT_SDA;
		    gpioPortSCL = I2C2_GPIO_PORT_SCL;
			gpioInit_SDA.GPIO_Pin = I2C2_SDA_PIN;
			gpioInit_SCL.GPIO_Pin = I2C2_SCL_PIN;
			RCC_AHB1PeriphClockCmd(I2C2_GPIO_SDA_RCC, ENABLE);
			RCC_AHB1PeriphClockCmd(I2C2_GPIO_SCL_RCC, ENABLE);
			GPIO_PinAFConfig(gpioPortSDA, I2C2_SDA_PIN_SOURCE, GPIO_AF_I2C2);
			GPIO_PinAFConfig(gpioPortSCL, I2C2_SCL_PIN_SOURCE, GPIO_AF_I2C2);
			break;
		
		case BSP_I2C_ID_3:
			gpioPortSDA = I2C3_GPIO_PORT_SDA;
		    gpioPortSCL = I2C3_GPIO_PORT_SCL;
			gpioInit_SDA.GPIO_Pin = I2C3_SDA_PIN;
			gpioInit_SCL.GPIO_Pin = I2C3_SCL_PIN;
			RCC_AHB1PeriphClockCmd(I2C3_GPIO_SDA_RCC, ENABLE);
			RCC_AHB1PeriphClockCmd(I2C3_GPIO_SCL_RCC, ENABLE);
			GPIO_PinAFConfig(gpioPortSDA, I2C3_SDA_PIN_SOURCE, GPIO_AF_I2C3);
			GPIO_PinAFConfig(gpioPortSCL, I2C3_SCL_PIN_SOURCE, GPIO_AF_I2C3);
			break;
		
		default:
			break;
	}
	
	gpioInit_SDA.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_SDA.GPIO_OType = GPIO_OType_OD;
	gpioInit_SDA.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit_SDA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortSDA, &gpioInit_SDA);
	
	gpioInit_SCL.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit_SCL.GPIO_OType = GPIO_OType_OD;
	gpioInit_SCL.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit_SCL.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(gpioPortSCL, &gpioInit_SCL);
}


/*
*********************************************************************************************************
*                                        NVIC_Configuration()
*
* Description : 初始化 I2C 链接到NVIC
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2C_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  NVIC_Configuration (CPU_INT08U i2c_id)
{
	NVIC_InitTypeDef nvicInitEV, nvicInitER;
  
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return;
	
	switch (i2c_id) {                                                           /* 配置中断源                      */
		case BSP_I2C_ID_1:
			nvicInitEV.NVIC_IRQChannel = BSP_I2C1_EV_IRQ;
			nvicInitER.NVIC_IRQChannel = BSP_I2C1_ER_IRQ;
			break;
		
		case BSP_I2C_ID_2:
			nvicInitEV.NVIC_IRQChannel = BSP_I2C2_EV_IRQ;
			nvicInitER.NVIC_IRQChannel = BSP_I2C2_ER_IRQ;
			break;
		
		case BSP_I2C_ID_3:
			nvicInitEV.NVIC_IRQChannel = BSP_I2C3_EV_IRQ;
			nvicInitER.NVIC_IRQChannel = BSP_I2C3_ER_IRQ;
			break;
		
		default:
			break;
	}         

	nvicInitEV.NVIC_IRQChannelPreemptionPriority = BSP_I2C_NVIC_PRE_PRIO;         /* 抢断优先级                      */
	nvicInitEV.NVIC_IRQChannelSubPriority        = BSP_I2C_NVIC_SUB_PRIO;         /* 子优先级                        */
	nvicInitEV.NVIC_IRQChannelCmd                = ENABLE;                        /* 使能中断                        */
	NVIC_Init(&nvicInitEV);
	
	nvicInitER.NVIC_IRQChannelPreemptionPriority = BSP_I2C_NVIC_PRE_PRIO;         /* 抢断优先级                      */
	nvicInitER.NVIC_IRQChannelSubPriority        = BSP_I2C_NVIC_SUB_PRIO;         /* 子优先级                        */
	nvicInitER.NVIC_IRQChannelCmd                = ENABLE;                        /* 使能中断                        */
	NVIC_Init(&nvicInitER);
}



/*
*********************************************************************************************************
*                                        BSP_I2C_SEM_Creat()
*
* Description : 根据id创建相应的信号量
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : DEF_OK     创建成功
*               DEF_FAIL   创建失败
*
* Caller(s)   : BSP_I2C_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_I2C_SEM_Creat (CPU_INT08U i2c_id)
{
	CPU_ERR  err;
	BSP_I2C_PERIPH_STATUS *currentStatusPtr;
	
	if(i2c_id >= BSP_I2C_NBR_MAX)
		return DEF_FAIL;
	
	currentStatusPtr = i2c_id+BSP_I2C_PeriphTbl;
	
	OSSemCreate((OS_SEM    *)&(currentStatusPtr->SemLock),
                (CPU_CHAR  *)"I2C SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	OSSemCreate((OS_SEM    *)&(currentStatusPtr->SemWait),
                (CPU_CHAR  *)"I2C SemWait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
	
		if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	return DEF_OK;
}


/*
*********************************************************************************************************
*                                       BSP_I2C_StartXfer()
*
* Description : Initialize and Start a new transfer in the I2C bus.
*
* Argument(s) : i2c_id             BSP_I2C_ID_x   x=[1,3]
*
*               i2c_addr           The I2C device address
*
*               i2c_acess_type     I2C Access Type
*                                      BSP_I2C_ACCESS_TYPE_RD
*                                      BSP_I2C_ACCESS_TYPE_WR
*                                      BSP_I2C_ACCESS_TYPE_WR_RD
*                               
*               p_buf              Pointer to the buffer into which the bytes will be stored.
*
*               nbr_bytes          Number of bytes to read.
*
* Return(s)   : DEF_OK            If the transfer could be initialized and started 
*               DEF_FAIL          If the transfer could no bet initialized and started
*
* Caller(s)   : BSP_I2C_Rd()
*               BSP_I2C_Wr()
*               BSP_I2C_WrRd()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  CPU_BOOLEAN  BSP_I2C_StartXfer (CPU_INT08U              i2c_id,
                                        CPU_INT08U              i2c_addr,
                                        I2C_AccessType_TypeDef  i2c_access_type,
                                        CPU_INT08U             *p_buf,                    
                                        CPU_INT16U              nbr_bytes)
{
	CPU_BOOLEAN           state = DEF_FAIL;
	OS_ERR                err;
	I2C_TypeDef           *i2cPort;
	BSP_I2C_PERIPH_STATUS *i2cPeriphStatusPtr;                                          //当前端口状态指针

	if(0 == nbr_bytes)                                                                  //参数检查
		return state;

    if(0 == p_buf)
        return state;
    
	switch (i2c_id) {                                                                   //选定端口号
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			break;
		
		default:
			return DEF_FAIL;                                                            //参数判定
	}
	
	i2cPeriphStatusPtr = BSP_I2C_PeriphTbl+i2c_id;                                      //对应到当前端口使用的状态结构
	
	OSSemPend((OS_SEM *)&(i2cPeriphStatusPtr->SemLock),                                 //锁定该端口
	          (OS_TICK ) BSP_I2C_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
	i2cPeriphStatusPtr->AccessType = i2c_access_type;
	i2cPeriphStatusPtr->Addr       = i2c_addr;
	i2cPeriphStatusPtr->BufLen     = nbr_bytes;
	i2cPeriphStatusPtr->BufPtr     = p_buf;
	i2cPeriphStatusPtr->State      = BSP_I2C_STATE_START; 
	
	I2C_ITConfig(i2cPort, I2C_IT_EVT, ENABLE);                                          //使能中断
	I2C_ITConfig(i2cPort, I2C_IT_ERR, ENABLE);
	I2C_GenerateSTART(i2cPort,ENABLE);                                                  //发送起始信号
	
	OSSemPend((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                                 //锁定该端口
	          (OS_TICK ) BSP_I2C_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(0 == i2cPeriphStatusPtr->BufLen)                                                //表示非异常
		state = DEF_OK;

	OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemLock),
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
	return state;
}




/*
*********************************************************************************************************
*                                        BSP_I2Cx_EV_Handler()
*
* Description : 根据id处理响应的事件
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2Cx_EV_Handler()     x=[1,3]
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_I2Cx_EV_Handler (CPU_INT08U i2c_id)
{
	OS_ERR                err;
	I2C_TypeDef           *i2cPort;
	BSP_I2C_PERIPH_STATUS *i2cPeriphStatusPtr;                                          //当前端口状态指针
	
	switch (i2c_id) {                                                                   //选定端口号
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			break;
		
		default:
			return;                                                                     //参数判定
	}
	
	i2cPeriphStatusPtr = BSP_I2C_PeriphTbl+i2c_id;                                      //对应到当前端口使用的状态结构
	
	switch(i2cPeriphStatusPtr->State) {
		case BSP_I2C_STATE_START:   
			if(SET == I2C_GetITStatus(i2cPort, I2C_IT_SB)) {                            //起始位发送后事件
				if(BSP_I2C_ACCESS_TYPE_RD == i2cPeriphStatusPtr->AccessType) {          //端口读访问
					I2C_Send7bitAddress(i2cPort, 
					                    i2cPeriphStatusPtr->Addr, 
					                    I2C_Direction_Receiver);                        //发送读地址
				} else {                                                                                            
					I2C_Send7bitAddress(i2cPort, 
					                    i2cPeriphStatusPtr->Addr, 
					                    I2C_Direction_Transmitter);                     //发送写地址
				}
				i2cPeriphStatusPtr->State = BSP_I2C_STATE_ADDR;                         //进入地址已发送状态
			}
			break;
		
		case BSP_I2C_STATE_ADDR:
			if(SET == I2C_GetITStatus(i2cPort, I2C_IT_ADDR)) {                          //地址发送后从机响应ACK事件
				switch (i2cPeriphStatusPtr->AccessType) {
					case BSP_I2C_ACCESS_TYPE_RD:
						if(i2cPeriphStatusPtr->BufLen > 2) {                            //接收大于两个字节要接收时才使能ACK
                            I2C_AcknowledgeConfig(i2cPort, ENABLE);                     
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_DATA;             //状态切换准备接受数据
						} else { 
                            if(2 == i2cPeriphStatusPtr->BufLen) {                           //表示只接受两个数据
							    I2C_AcknowledgeConfig(i2cPort, DISABLE);                    //失能应答
							    I2C_NACKPositionConfig(i2cPort,I2C_NACKPosition_Next);      //只接受两个字节时必须置位POS
							    i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;             //切换到结束阶段参见STM32F4参考手册
                             } else {                                                       //只读取一个字节,注意不能在这里置位STOP,手册此处有点模糊
                                I2C_ITConfig(i2cPort,I2C_IT_BUF, ENABLE);                   //必须使能TXE中断，因为单字节接收不能置位BTF
                                I2C_AcknowledgeConfig(i2cPort, DISABLE);                    //失能应答
                                i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;
                             }
						} 
						I2C_ReadRegister(i2cPort, I2C_Register_SR2);                        //清零ADDR标志位,此处按照参考手册顺序	
						break;
 
					case BSP_I2C_ACCESS_TYPE_WR_RD:
						I2C_ReadRegister(i2cPort, I2C_Register_SR2);                        //在发送数据前必须先清零ADDR标志位，否则不能进入中断	
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
							I2C_SendData(i2cPort, *(i2cPeriphStatusPtr->BufPtr));           //发送第一个数据
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_DATA;
						}
						break;
					
					case BSP_I2C_ACCESS_TYPE_WR:
						I2C_ReadRegister(i2cPort, I2C_Register_SR2);                        //在发送数据前必须先清零ADDR标志位，否则不能进入中断	
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
							I2C_SendData(i2cPort, *(i2cPeriphStatusPtr->BufPtr));           //发送第一个数据
							i2cPeriphStatusPtr->BufLen--;
							i2cPeriphStatusPtr->BufPtr++;
							if(0 != i2cPeriphStatusPtr->BufLen) {                           //判断是否只发送一个字节
								i2cPeriphStatusPtr->State = BSP_I2C_STATE_DATA;
							} else {                                                        
								i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;   
							}
						}
						break;
						
					default:
						break;
				}	
			} else {                                                                   //非预期情况处理
				i2cPeriphStatusPtr->State      = BSP_I2C_STATE_IDLE;
				i2cPeriphStatusPtr->AccessType = BSP_I2C_ACCESS_TYPE_NONE;	
                I2C_ITConfig(i2cPort,I2C_IT_EVT, DISABLE);
			    I2C_ITConfig(i2cPort,I2C_IT_ERR, DISABLE);
                I2C_ITConfig(i2cPort,I2C_IT_BUF, DISABLE);
                I2C_GenerateSTOP(i2cPort, ENABLE);
                OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                    //释放信号量
	                      (OS_OPT  )OS_OPT_POST_1,
	                      (OS_ERR *) &err);
			}
			break;
		
		case BSP_I2C_STATE_DATA:
			switch (i2cPeriphStatusPtr->AccessType) {
				case BSP_I2C_ACCESS_TYPE_RD:	
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_BTF)) {
						*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);      //读取数据
						i2cPeriphStatusPtr->BufLen--;
						i2cPeriphStatusPtr->BufPtr++; 
						if(2 == i2cPeriphStatusPtr->BufLen) {                          //表示还能接收最后两个字节
							I2C_AcknowledgeConfig(i2cPort, DISABLE);
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;
						} 
					}
					break;
				
				case BSP_I2C_ACCESS_TYPE_WR:
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
						I2C_SendData(i2cPort, *(i2cPeriphStatusPtr->BufPtr));          //写数据
						i2cPeriphStatusPtr->BufLen--;
						i2cPeriphStatusPtr->BufPtr++;
						if(0 == i2cPeriphStatusPtr->BufLen) {
							i2cPeriphStatusPtr->State = BSP_I2C_STATE_STOP;            //表示结束
						}
					}
					break;

                case BSP_I2C_ACCESS_TYPE_WR_RD:
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
                        i2cPeriphStatusPtr->AccessType = BSP_I2C_ACCESS_TYPE_RD;       //过渡到读模式
                        i2cPeriphStatusPtr->State      = BSP_I2C_STATE_START;          //准备进入开始状态
						I2C_GenerateSTART(i2cPort, ENABLE);
					}
					break;
					
				default:
					break;
			}
			break;
         		
   		case BSP_I2C_STATE_STOP:
   			switch (i2cPeriphStatusPtr->AccessType) {
   				case BSP_I2C_ACCESS_TYPE_RD:
					if(2 == i2cPeriphStatusPtr->BufLen) {                                 //常规结束方式一次读取两个数据参见手册
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_BTF)) {
							I2C_GenerateSTOP(i2cPort, ENABLE);                            //按照手册顺序置位STOP
							*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);     //读取倒数第二个字节
							i2cPeriphStatusPtr->BufPtr++;      
							*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);     //读取最后一个字节
							i2cPeriphStatusPtr->BufLen -= 2;                              //递减缓冲区长度
						}
					} else {                                                              //只有读取单字节时才会发生
						if(SET == I2C_GetITStatus(i2cPort, I2C_IT_RXNE)) {                //单字节模式不会置位BTF
							I2C_GenerateSTOP(i2cPort, ENABLE);                            
							*(i2cPeriphStatusPtr->BufPtr) = I2C_ReceiveData(i2cPort);      
							i2cPeriphStatusPtr->BufLen--;       
							I2C_ITConfig(i2cPort,I2C_IT_BUF, DISABLE);                    //失能TXE与RXNE中断
						}
					}
					break;
		
			    case BSP_I2C_ACCESS_TYPE_WR:
					if(SET == I2C_GetITStatus(i2cPort, I2C_IT_TXE)) {
						I2C_GenerateSTOP(i2cPort, ENABLE);
					}
					break;
			
				default:
					break;	                 
			}

			OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                           //通知读写函数读取缓冲区
	                  (OS_OPT  )OS_OPT_POST_1,
	                  (OS_ERR *) &err);
			
			/*------------------------  关闭中断  ----------------------------*/
			I2C_ITConfig(i2cPort,I2C_IT_EVT, DISABLE);
			I2C_ITConfig(i2cPort,I2C_IT_ERR, DISABLE);
			break;
			
	default:
		break;
	}
}




/*
*********************************************************************************************************
*                                        BSP_I2Cx_ER_Handler()
*
* Description : 根据id处理响应的错误，即将端口的状态恢复，但是不会清BufLen
*
* Argument(s) : i2c_id     BSP_I2C_ID_x   x=[1,3]
*
* Return(s)   : none
*
* Caller(s)   : BSP_I2Cx_ER_Handler()     x=[1,3]
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  BSP_I2Cx_ER_Handler (CPU_INT08U i2c_id)
{
	OS_ERR                err;
	CPU_INT32U            status;
	I2C_TypeDef           *i2cPort;
	BSP_I2C_PERIPH_STATUS *i2cPeriphStatusPtr;                                        //当前端口状态指针
	
	switch (i2c_id) {                                                                 //选定端口号
		case BSP_I2C_ID_1:
			i2cPort = I2C1;
			break;
		
		case BSP_I2C_ID_2:
			i2cPort = I2C2;
			break;
		
		case BSP_I2C_ID_3:
			i2cPort = I2C3;
			break;
		
		default:
			return;                                                                   //参数判定
	}
	
	i2cPeriphStatusPtr = BSP_I2C_PeriphTbl+i2c_id; 
	
	status  = I2C_ReadRegister(i2cPort, I2C_Register_SR1);                            //读取SR1
	status &= BSP_I2C_REG_SR1_ERR_MASK;                                               //只保留错误位   
	i2cPort->SR1 &= ~status;                                                          //清零错误标志位

	 if (i2cPeriphStatusPtr->State != BSP_I2C_STATE_IDLE) {
        i2cPeriphStatusPtr->State      = BSP_I2C_STATE_IDLE;
        i2cPeriphStatusPtr->AccessType = BSP_I2C_ACCESS_TYPE_NONE;
		 
		I2C_GenerateSTOP(i2cPort, ENABLE);
                                       
		OSSemPost((OS_SEM *)&(i2cPeriphStatusPtr->SemWait),                           //释放信号量                    
	              (OS_OPT  )OS_OPT_POST_1,
	              (OS_ERR *) &err);
		 
		/*------------------------  关闭中断  ----------------------------*/
		I2C_ITConfig(i2cPort,I2C_IT_EVT, DISABLE);
        I2C_ITConfig(i2cPort,I2C_IT_BUF, DISABLE);
		I2C_ITConfig(i2cPort,I2C_IT_ERR, DISABLE);        	
    }
}



/*
*********************************************************************************************************
*                                              I2C1系统事件中断函数与错误中断函数
*********************************************************************************************************
*/

void BSP_I2C1_EV_Handler(void)
{
	BSP_I2Cx_EV_Handler(BSP_I2C_ID_1);
}

void BSP_I2C1_ER_Handler(void)
{
	BSP_I2Cx_ER_Handler(BSP_I2C_ID_1);
}


/*
*********************************************************************************************************
*                                              I2C2系统事件中断函数与错误中断函数
*********************************************************************************************************
*/

void BSP_I2C2_EV_Handler(void)
{
	BSP_I2Cx_EV_Handler(BSP_I2C_ID_2);
}

void BSP_I2C2_ER_Handler(void)
{
	BSP_I2Cx_ER_Handler(BSP_I2C_ID_2);
}



/*
*********************************************************************************************************
*                                              I2C3系统事件中断函数与错误中断函数
*********************************************************************************************************
*/

void BSP_I2C3_EV_Handler(void)
{
	BSP_I2Cx_EV_Handler(BSP_I2C_ID_3);
}

void BSP_I2C3_ER_Handler(void)
{
	BSP_I2Cx_ER_Handler(BSP_I2C_ID_3);
}


