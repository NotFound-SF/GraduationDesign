
/*
*********************************************************************************************************
*      因为只接触过该型号的SRAM,所以该款驱动只保证适用于 IS62WV51216BLL 55ns版
*********************************************************************************************************
*/


#include "bsp_sram.h"


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void         BSP_SRAM_CTRL_GPIO_Init (void);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAl FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         BSP_SRAM_Init()
*
* Description :  初始化SRAM,其中包含初始化FSCM
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_SRAM_Init(void)
{
	FSMC_NORSRAMInitTypeDef        sramInit;
	FSMC_NORSRAMTimingInitTypeDef  sramTimigRd;
	
	// 初始GPIO通用引脚和控制SRAM的独特控制引脚
	
	BSP_SRAM_CTRL_GPIO_Init();                                                
	
	FSMC_NORSRAMDeInit(BSP_SRAM_Bank);
	
	// 读时序配置
	
	sramTimigRd.FSMC_AccessMode            = FSMC_AccessMode_A;                //模式A参见参考手册
	sramTimigRd.FSMC_DataSetupTime         = 0x08;                             // x/55ns = 168/1000 ns ,x = 9.24
	sramTimigRd.FSMC_AddressSetupTime      = 0x03;                             //地址建立时间
	sramTimigRd.FSMC_CLKDivision           = 0x00;                             //SRAM工作在异步模式该位无意义                      
	sramTimigRd.FSMC_DataLatency           = 0x00;                             //表示数据延迟周期，SRAM工作在异步模式该位无意义 
	sramTimigRd.FSMC_AddressHoldTime       = 0x00;                             //使用与模式D,模式A该位无意义
	sramTimigRd.FSMC_BusTurnAroundDuration = 0x01;                             //SRAM 该位无意义
	
	sramInit.FSMC_MemoryType            = FSMC_MemoryType_SRAM;                //存储器类型
	sramInit.FSMC_Bank                  = BSP_SRAM_Bank;                       //SRAM所在块
	sramInit.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;         //不复用地址引脚与数据引脚
	sramInit.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;          //允许写访问
	sramInit.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;           //读写时序可以分别配置
	sramInit.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;            //外部存储器数据宽度
	
	sramInit.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;        //仅适用于同步存储器
	sramInit.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;       //该SRAM无等待引脚
	sramInit.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;         //等待信号为高电平有效，SRAM无用
	sramInit.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;             //用于NOR
	sramInit.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	sramInit.FSMC_WrapMode              = FSMC_WrapMode_Disable;               //仅在突发模式下有效
	sramInit.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;             //仅在同步模式下有效
	
	sramInit.FSMC_ReadWriteTimingStruct = &sramTimigRd;
	sramInit.FSMC_WriteTimingStruct     = &sramTimigRd;
	
	FSMC_NORSRAMInit(&sramInit);
	FSMC_NORSRAMCmd(BSP_SRAM_Bank, ENABLE);
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
*                                         BSP_SRAM_CTRL_GPIO_Init()
*
* Description :  初始化SRAM控制引脚
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_SRAM_GPIO_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/
static  void  BSP_SRAM_CTRL_GPIO_Init (void)
{
	GPIO_InitTypeDef gpioInit;
	                                                  
	// 开启控制引脚端口时钟
	
	RCC_AHB1PeriphClockCmd(SRAM_GPIO_PORT_RCC_NCE, ENABLE);
	RCC_AHB1PeriphClockCmd(SRAM_GPIO_PORT_RCC_NLB, ENABLE);
	RCC_AHB1PeriphClockCmd(SRAM_GPIO_PORT_RCC_NUB, ENABLE);
																			   
	// 复用控制引脚
	
	GPIO_PinAFConfig(SRAM_GPIO_PORT_NCE, SRAM_PIN_SOURCE_NCE, GPIO_AF_FSMC);
	GPIO_PinAFConfig(SRAM_GPIO_PORT_NLB, SRAM_PIN_SOURCE_NLB, GPIO_AF_FSMC);
	GPIO_PinAFConfig(SRAM_GPIO_PORT_NUB, SRAM_PIN_SOURCE_NUB, GPIO_AF_FSMC);
		                                                                      
	 // 配置控制引脚模式
	
	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_UP;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// 初始化NCE
	
	gpioInit.GPIO_Pin   = SRAM_PIN_NCE;                                        
	GPIO_Init(SRAM_GPIO_PORT_NCE, &gpioInit);
	
	 // 初始化NLB
	
	gpioInit.GPIO_Pin   = SRAM_PIN_NLB;                                       
	GPIO_Init(SRAM_GPIO_PORT_NLB, &gpioInit);
	
	// 初始化NUB
	
	gpioInit.GPIO_Pin   = SRAM_PIN_NUB;                                        
	GPIO_Init(SRAM_GPIO_PORT_NUB, &gpioInit);
}



