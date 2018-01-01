
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_18b20.h"


/*
*********************************************************************************************************
*                                            LOCAL DATA
*********************************************************************************************************
*/

static      __IO           uint16_t tempBuf;                                      //缓存读取到的温度   


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void            BSP_18B20_GPIO_Init(void);
static  void            BSP_18B20_WriteByte(uint8_t data);
static  CPU_BOOLEAN     BSP_18B20_GetAck(void);
static  uint8_t         BSP_18B20_ReadByte(void);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        BSP_18B20_Init()
*
* Description : 初始化 GPIO映射到DS18B20
*
* Argument(s) : none
*
* Return(s)   : DEF_OK      初始化成功    
*               DEF_FAIL    初始化失败，未检测到DS18B20
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_18B20_Init(void)
{
	CPU_BOOLEAN status;
	
	BSP_18B20_GPIO_Init();                                //初始化端口
	
	// 确保该进程整个时序独占该定时器
	
	Timing_Lock();   
	
	if (0 == BSP_18B20_GetAck()) {
		status = DEF_OK;                                  //表示器件存在
		BSP_18B20_WriteByte(0xCC);                        //跳过ROM
		
		// 设置配置寄存器，精确到9Bit即0.5C'
		
		BSP_18B20_WriteByte(0x4E);                        //设置暂存器指令
		BSP_18B20_WriteByte(0xFF);                        //TH
		BSP_18B20_WriteByte(0xFF);                        //TL
		BSP_18B20_WriteByte(ACCURACY);                        //config寄存器
		
		BSP_18B20_WriteByte(0x44);                        //启动一次温度转换
	} else {
		status = DEF_FAIL;                                //表示器件不存在
	}
	
	// 释放定时器
	Timing_Unlock();                                  //进程完整时序结束释放定时器
	
	return status;
}



/*
*********************************************************************************************************
*                                        BSP_18B20_GetTemp()
*
* Description : 读取DS18B20的温度值，其中有较长的转换延时
*
* Argument(s) : temp        读取温度值
*
* Return(s)   : DEF_OK      读取温度成功    
*               DEF_FAIL    读取温度失败
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_18B20_GetTemp(uint16_t *ptemp)
{
	uint16_t      temp;
	CPU_BOOLEAN   status;
	
	// 确保该进程整个时序独占该定时器
	
	Timing_Lock();  
	
	// 读取温度前先转换一次
	
	if (0 == BSP_18B20_GetAck()) {
		BSP_18B20_WriteByte(0xCC);                        //跳过ROM
		BSP_18B20_WriteByte(0x44);                        //启动一次温度转换
		Timing_Delay_Ms(CONVERT_T);                       //转换数据需要时间
	} 
	
	// 完整的温度读取操作
	if (0 == BSP_18B20_GetAck()) {
		status = DEF_OK;
		BSP_18B20_WriteByte(0xCC);                       //跳过ROM	
		BSP_18B20_WriteByte(0xBE);                       //发送读温度命令
		temp  = BSP_18B20_ReadByte();                    //读温度低字节
		temp |= (uint16_t)BSP_18B20_ReadByte()<<8;       //读温度高字节
		tempBuf = temp;
		*ptemp = temp;
	} else {
		status = DEF_FAIL;
	}
	
	// 释放定时器
	Timing_Unlock();                                     //进程完整时序结束释放定时器
	
	return status;
}


/*
*********************************************************************************************************
*                                        BSP_18B20_GetTempFast()
*
* Description : 读取DS18B20的温度值，但是读取到温度并不是最新的温度，而是缓冲区的温度值(上次读取的温度值)
*               所以不会产生通信时序而是直接返回缓冲区的数值；
*
* Argument(s) : none
*
* Return(s)   : 16位18B20格式的原始数据
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint16_t   BSP_18B20_GetTempFast(void)
{
		return tempBuf;
}


/*
*********************************************************************************************************
*                                        BSP_18B20_TempTran()
*
* Description : 将18B20类型数据转换为浮点型
*
* Argument(s) : temp        DS18B20格式温度值
*
* Return(s)   : 浮点类型的温度值
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

float BSP_18B20_TempTran(uint16_t temp)
{
	float  dat;
	
	
	// 获取温度的实际数值，不包含符号位
	
	dat = (temp >> 4)&0x7F;                         //提取整数部分
	dat += (float)(temp&0x0F) / 16;                 //提取小数部分
	
	// 判断温度的符号
	
	if (0 != (temp&0xF800)) {                       //判断符号为，全为1表示零下温度值
		return -dat;
	} 
	
	return dat;
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
*                                        BSP_18B20_GPIO_Init()
*
* Description : 初始化 DS18B20的GPIO引脚
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_18B20_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_18B20_GPIO_Init (void)
{
	GPIO_InitTypeDef  gpioInit;
	
	RCC_AHB1PeriphClockCmd(DS18B20_GPIO_CLK, ENABLE);        //开启时钟
	
	gpioInit.GPIO_Pin   = DS18B20_PIN;
	gpioInit.GPIO_Mode  = GPIO_Mode_OUT;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_OType = GPIO_OType_OD;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(DS18B20_GPIO_PORT, &gpioInit);                 //初始化引脚
}


/*
*********************************************************************************************************
*                                        BSP_18B20_GetAck()
*
* Description : 获取总线上DS18B20存在信息
*
* Argument(s) : none
*
* Return(s)   : 0      测到DS18B20     
*               1      未检测到DS18B20
*
* Caller(s)   : BSP_18B20_Init(), BSP_18B20_GetTemp()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static CPU_BOOLEAN BSP_18B20_GetAck(void)
{
	CPU_BOOLEAN status;
	                                
	// 由主机发出复位脉冲即480~960us低电平
	
	DS18B20_PIN_OUT();                                // 确保是在输出模式
	DS18B20_PIN_WR_0();                               // 设置引脚为低电平
	Timing_Delay_Us(500);
	
	// 主机释放总线等待18B20应答
	
	DS18B20_PIN_IN();                                 //设置为输入模式释放总线
	Timing_Delay_Us(60);                              
	status = DS18B20_PIN_RD();                        //读取引脚状态
	
	//等待18B20应答结束
	
	Timing_Delay_Us(240);                             	
	
	return status;                                   
}


/*
*********************************************************************************************************
*                                        BSP_18B20_WriteByte()
*
* Description : 主机向DS18B20写入一个字节
*
* Argument(s) : data    要写入的数据或者命令
*
* Return(s)   : none
*
* Caller(s)   : BSP_18B20_Init(), BSP_18B20_GetTemp()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_18B20_WriteByte(uint8_t data)
{
	uint8_t mask;
	
	DS18B20_PIN_OUT();                                    // 确保是在输出模式
	
	for(mask = 0x01; mask != 0; mask <<= 1) {
		// 保证每次开始都是高电平
		DS18B20_PIN_WR_1();     
		Timing_Delay_Us(1);                               //两个bit之间的间隔必须保证大于1us
		
		// 主机产生2us的低电平
		
		DS18B20_PIN_WR_0();                               // 设置引脚为低电平
		Timing_Delay_Us(2);
		
		// 主机根据bit值决策下一步
		
		if(mask & data) 
			DS18B20_PIN_WR_1();                           //写1
		else 
			DS18B20_PIN_WR_0();                           //写0
		
		Timing_Delay_Us(58);                              //延迟58us
	}
	
	DS18B20_PIN_WR_1();                                   //每次写完保持总线为高电平
	
}


/*
*********************************************************************************************************
*                                        BSP_18B20_ReadByte()
*
* Description : 从DS18B20读取一个字节
*
* Argument(s) : none
*
* Return(s)   : 读取的数据
*
* Caller(s)   : BSP_18B20_GetTemp()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static uint8_t  BSP_18B20_ReadByte(void)
{
	uint8_t mask;
	uint8_t data = 0x00;                                        // 不初始化就会导致读取出错？？
	
	for(mask = 0x01; mask != 0; mask <<= 1) {
		
		// 产生读取每个bit的起始信号
		
		DS18B20_PIN_OUT();                                  // 确保是在输出模式
		DS18B20_PIN_WR_0();                   
		Timing_Delay_Us(2);   
		
		//进入读取状态
		
		DS18B20_PIN_IN();   
		Timing_Delay_Us(3);                                //必须在15us以内采样   
		
		if (DS18B20_PIN_RD())                                 
			data |= mask;
		else	
			data  &= ~mask;
		
		Timing_Delay_Us(56);                               //确保完整周期为60us
	}
	
	return data;
}


