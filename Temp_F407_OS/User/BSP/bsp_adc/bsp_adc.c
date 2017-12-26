
/*
*********************************************************************************************************
*        该驱动主要是针对电流传感器，MQ-2气体传感器，光照强度三个需要用到AD的传感器编写
*        所以该驱动只开启一个ADC并且开启3个通道，该驱动函数只调用一次,在调用该函数后ADC
*        就开始默默的再后台转换，数据自动更新，只提供读取接口，所以不需要信号量来互斥
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_adc.h"

/*
*********************************************************************************************************
*                                              LOCAL DATA
*********************************************************************************************************
*/

static   __IO   uint16_t  ADC_Value[DAC_CH_NUM];                           //存放ADC读取的数据，由DMA更新               

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static           void            BSP_ADC_DMA_Config(void);
static           void            BSP_ADC_GPIO_Config(void);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_ADC_Init()
*
* Description : 配置DMA，配置IO引脚，将DMA链接到内存与ADC，开启三个通道
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

void   BSP_ADC_Init(void)
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
	// 开启ADC外设时钟
	
	ADC_RCC_CMD(ADC_CLK, ENABLE);
	
	// 初始DMA与GPIO
	
	BSP_ADC_DMA_Config();
	BSP_ADC_GPIO_Config();
	
		
	// ADC Common 结构体 参数 初始化
	
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                       // 独立ADC模式
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;                    // 时钟为fpclk x分频
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;        // 禁止DMA直接访问模式	
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;  // 采样时间间隔	实用与双重模式
	
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	// ADC_InitTypeDef结构体初始化
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                         // 12位分辨率
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                                   // 多通道采集才需要	
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                             // 连续转换，只需要触发一次就开始连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;    // 不使用外部触发
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;          // 因为不使用外部触发所以无效
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                         // 数据右对齐	
	ADC_InitStructure.ADC_NbrOfConversion = DAC_CH_NUM;                            // 转换通道 3个     
	
	ADC_Init(ADC_PORT, &ADC_InitStructure);
	//---------------------------------------------------------------------------
	
	// 配置 ADC 通道转换顺序为1，第一个转换，采样时间为56个时钟周期                    
	
	ADC_RegularChannelConfig(ADC_PORT, DAC_CH1, 1, ADC_SampleTime_112Cycles);      //第一个转换
	ADC_RegularChannelConfig(ADC_PORT, DAC_CH2, 2, ADC_SampleTime_112Cycles);      //第二个转换
	ADC_RegularChannelConfig(ADC_PORT, DAC_CH3, 3, ADC_SampleTime_112Cycles);      //第三个转换

	
	ADC_DMARequestAfterLastTransferCmd(ADC_PORT, ENABLE);
    ADC_DMACmd(ADC_PORT, ENABLE);                                                  //使能ADC DMA请求
	ADC_Cmd(ADC_PORT, ENABLE);                                                     //使能ADC
	ADC_SoftwareStartConv(ADC_PORT);                                               //软件触发转换
}



/*
*********************************************************************************************************
*                                        BSP_ADC_GetDat()
*
* Description : 根据通道编号返回对应通道的数据
*
* Argument(s) : CHx  BSP_ADC_CH1      通道1
*                    BSP_ADC_CH2      通道2
*                    BSP_ADC_CH3      通道3
*
* Return(s)   : ADC的数数据,未经过换算
*
* Caller(s)   : Application   
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint16_t BSP_ADC_GetDat(BSP_ADC_CHx CHx)
{
	uint16_t dat;
	 
	switch (CHx) {
		
		case BSP_ADC_CH1:
			dat = ADC_Value[0];
			break;
		
		case BSP_ADC_CH2:
			dat = ADC_Value[1];
			break;
	
		case BSP_ADC_CH3:
			dat = ADC_Value[2];
			break;
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
*                                        BSP_ADC_DMA_Config()
*
* Description : 配置DMA，将DMA链接到内存与ADC
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_ADC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_ADC_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(ADC_DMA_CLK, ENABLE);      			                     //开启DMA时钟

	DMA_InitStructure.DMA_Channel = ADC_DMA_CHANNEL;
	DMA_InitStructure.DMA_BufferSize = DAC_CH_NUM;            		                 //缓存区大小
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;    			             //关闭FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC_Value;                     //内存基地址
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                      //单数据发送
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                          //内存地址增
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                  //循环模式才能开启双缓冲模式
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC_DR_ADDR;   
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;      //两个字节传输
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;                               //配置优先级
	
	
	DMA_Init(ADC_DMA_STREAM, &DMA_InitStructure);
	DMA_Cmd(ADC_DMA_STREAM, ENABLE);
}



/*
*********************************************************************************************************
*                                        ADC_DMA_Config()
*
* Description : 将GPIO映射到ADC
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_ADC_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void BSP_ADC_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能 GPIO 时钟
	RCC_AHB1PeriphClockCmd(DAC_CH1_GPIO_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(DAC_CH2_GPIO_RCC, ENABLE);
	RCC_AHB1PeriphClockCmd(DAC_CH3_GPIO_RCC, ENABLE);
		
	// 配置 IO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;                    //不上拉不下拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = DAC_CH1_PIN;
	GPIO_Init(DAC_CH1_GPIO_PORT, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = DAC_CH2_PIN;
	GPIO_Init(DAC_CH2_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = DAC_CH3_PIN;
	GPIO_Init(DAC_CH3_GPIO_PORT, &GPIO_InitStructure);
}












