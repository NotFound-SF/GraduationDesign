
/*
*********************************************************************************************************
*      因为接触的LCD较少所以该驱动只适用与ILI9341驱动的LCD并且必须是8080接口
*********************************************************************************************************
*/

#include "bsp_lcd.h"


//根据液晶扫描方向而变化的XY像素宽度
//调用ILI9341_GramScan函数设置方向时会自动更改
uint16_t LCD_X_LENGTH = ILI9341_LESS_PIXEL;
uint16_t LCD_Y_LENGTH = ILI9341_MORE_PIXEL;


//液晶屏扫描模式，本变量主要用于方便选择触摸屏的计算参数
//参数可选值为0-7
//调用ILI9341_GramScan函数设置方向时会自动更改
//LCD刚初始化完成时会使用本默认值

uint8_t LCD_SCAN_MODE = 3;    //横屏
//uint8_t LCD_SCAN_MODE = 6;      //竖屏


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static __inline    void       BSP_LCD_WriteCmd(uint16_t cmd);
static __inline    void       BSP_LCD_WriteDat(uint16_t dat);

static  void         BSP_LCD_CTRL_GPIO_Init (void);
static  void         BSP_LCD_FSMC_Init      (void);
static  void         ILI9341_REG_Config     (void);
static  void         ILI9341_GramScan       (uint8_t ucOption);
static  void         BSP_LCD_SetCursor      (uint16_t usX, uint16_t usY);
static  void         BSP_LCD_OpenWindow     (uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);

#if  LCD_DMA_EN > 0	
static  void         BSP_LCD_DMA_Config     (void);
#endif /* LCD_DMA_EN */



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAl FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         BSP_LCD_Init()
*
* Description :  初始化LCD的片选引脚与背光引脚
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void  BSP_LCD_Init(void)
{
	// 初始化FSMC与初始化控制引脚
	BSP_LCD_CTRL_GPIO_Init();
	
	// 初始化FSMC
	
	BSP_LCD_FSMC_Init();
	
	// 初始化LCD控制器
	
	ILI9341_REG_Config();
	
	// 配置DMA
#if  LCD_DMA_EN > 0	
	BSP_LCD_DMA_Config();
#endif /* LCD_DMA_EN */
	
	// 配置横竖屏模式
	
	ILI9341_GramScan(LCD_SCAN_MODE);
	
	BSP_LCD_BL_ON();                               // 开背光
	BSP_LCD_ClrScr(WHITE);                         // 清屏为白色
}

/*
*********************************************************************************************************
*                                         BSP_LCD_ClrScr()
*
* Description :  清屏函数
*
* Argument(s) :  usColor: 将屏幕清成制定颜色
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_LCD_ClrScr(uint16_t usColor)
{
	uint32_t index;   
	uint32_t count = ILI9341_ALL_PIXEL/10;               //要填充像素点的个数/10,减少for循环次数
	
	BSP_LCD_OpenWindow(0, 0, LCD_X_LENGTH, LCD_Y_LENGTH);
	BSP_LCD_WriteCmd(CMD_SetPixel);                      // 填充数据指令
	
	// 这样优化能每次减少9条跳转指令与比较指令
	
	for (index = 0; index < count; index++) {
		BSP_LCD_WriteDat(usColor);                       // 填充颜色
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
		BSP_LCD_WriteDat(usColor); 
	}
	
}


/*
*********************************************************************************************************
*                                         LCD_GetHeight()
*
* Description :  获取液晶屏幕高度
*
* Argument(s) :  none
*
* Return(s)   :  晶屏幕高度
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/


uint16_t LCD_GetHeight(void)
{
	return LCD_Y_LENGTH;
}


/*
*********************************************************************************************************
*                                         LCD_GetWidth()
*
* Description :  获取液晶屏幕宽度
*
* Argument(s) :  none
*
* Return(s)   :  晶屏幕宽度
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/


uint16_t LCD_GetWidth(void)
{
	return LCD_X_LENGTH;
}



/*
*********************************************************************************************************
*                                         BSP_LCD_SetPointPixel()
*
* Description :  对ILI9341显示器的某一点以某种颜色进行填充
*
* Argument(s) :  usX     ：在特定扫描方向下该点的X坐标
*                usY     ：在特定扫描方向下该点的Y坐标
*                usColor ：待填充的颜色
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_LCD_SetPointPixel (uint16_t usX, uint16_t usY, uint16_t usColor)	
{	
		BSP_LCD_SetCursor ( usX, usY );                      // 设置光标
		BSP_LCD_WriteCmd(CMD_SetPixel);                      // 填充数据指令
		BSP_LCD_WriteDat(usColor);                           // 填充颜色
}



/*
*********************************************************************************************************
*                                         BSP_LCD_GetPointPixel()
*
* Description :  获取显示器的某一点的颜色
*
* Argument(s) :  usX     ：在特定扫描方向下该点的X坐标
*                usY     ：在特定扫描方向下该点的Y坐标
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/ 

uint16_t BSP_LCD_GetPointPixel ( uint16_t usX, uint16_t usY )
{
	uint16_t usR=0, usG=0, usB=0 ;

	
	BSP_LCD_SetCursor ( usX, usY );                      // 设置光标
	BSP_LCD_WriteCmd(CMD_GetPixel);                      // 读取数据指令
	
	usR = BSP_LCD_DAT;   	/*FIRST READ OUT DUMMY DATA*/
	
	usR = BSP_LCD_DAT;  	/*READ OUT RED DATA  */
	usB = BSP_LCD_DAT;  	/*READ OUT BLUE DATA*/
	usG = BSP_LCD_DAT;  	/*READ OUT GREEN DATA*/	
	
    return ( ( ( usR >> 11 ) << 11 ) | ( ( usG >> 10 ) << 5 ) | ( usB >> 11 ) );
}




/*
*********************************************************************************************************
*                                         BSP_LCD_DrawHLine()
*
* Description :  绘制一条水平线 （主要用于STemwin的接口函数）
*
* Argument(s) :  usX1    ：起始点X坐标
*                usY1    ：水平线的Y坐标
*                usX2    ：结束点X坐标
*                usColor : 颜色
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_LCD_DrawHLine(uint16_t usX1 , uint16_t usY1 , uint16_t usX2 , uint16_t usColor)
{
	uint16_t index, len = usX2-usX1+1;

	BSP_LCD_OpenWindow(usX1, usY1, len, 1);

	BSP_LCD_WriteCmd(CMD_SetPixel);                       // 填充数据指令

	// 写显存 
	
	for (index = 0; index < len; index++)
	{
		BSP_LCD_WriteDat(usColor);
	}
}




/*
*********************************************************************************************************
*                                         BSP_LCD_DrawVLine()
*
* Description :  绘制一条锤子平线 （主要用于STemwin的接口函数）
*
* Argument(s) :  usX1    ：起始点X坐标
*                usY1    ：水平线的Y坐标
*                usY2    ：结束点Y坐标
*                usColor : 颜色
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_LCD_DrawVLine(uint16_t usX1 , uint16_t usY1 , uint16_t usY2 , uint16_t usColor)
{
	uint16_t index, len = usY2-usY1+1;

	BSP_LCD_OpenWindow(usX1, usY1, 1, len);

	BSP_LCD_WriteCmd(CMD_SetPixel);                       // 填充数据指令

	// 写显存 
	
	for (index = 0; index < len; index++)
	{
		BSP_LCD_WriteDat(usColor);
	}
}





/*
*********************************************************************************************************
*                                         BSP_LCD_FillRect()
*
* Description :  填充矩形 （主要用于STemwin的接口函数）
*
* Argument(s) :  usX, usY：矩形左上角的坐标
*                usHeight：矩形的高度
*			     usWidth ：矩形的宽度
*                usColor : 颜色
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_LCD_FillRect(uint16_t usX, uint16_t usY, uint16_t usHeight, uint16_t usWidth, uint16_t usColor)
{
	uint32_t index;
	uint32_t count = usHeight * usWidth;

	BSP_LCD_OpenWindow(usX, usY,  usWidth, usHeight);

	BSP_LCD_WriteCmd(CMD_SetPixel);                       // 填充数据指令
	
	// 写显存
	
	for (index = 0; index < count; index++)
	{
		BSP_LCD_WriteDat(usColor);
	}
}



/*
*********************************************************************************************************
*                                         BSP_LCD_DrawHColorLine()
*
* Description :  绘制一条彩色水平线（主要用于STemwin的接口函数）
*
* Argument(s) :   usX1    ：起始点X坐标
*			      usY1    ：水平线的Y坐标
*			      usWidth ：直线的宽度
*			       pColor : 颜色缓冲区
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

void BSP_LCD_DrawHColorLine(uint16_t usX1 , uint16_t usY1, uint16_t usWidth, const uint16_t *pColor)
{
	uint16_t index;
	
	BSP_LCD_OpenWindow(usX1, usY1, usWidth, 1);
	
	BSP_LCD_WriteCmd(CMD_SetPixel);                       // 填充数据指令
	
	// 写显存
	
	for (index = 0; index < usWidth; index++)
	{
		BSP_LCD_WriteDat(*pColor++);
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
*                                         BSP_LCD_WriteDat()
*
* Description :  向显存写入数据
*
* Argument(s) :  dat   要写入的数据
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static __inline void BSP_LCD_WriteDat(uint16_t dat)
{
	BSP_LCD_DAT = dat;
}




/*
*********************************************************************************************************
*                                         BSP_LCD_WriteCmd()
*
* Description :  向显存写入命令
*
* Argument(s) :  cmd   要写入的指令
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static __inline void BSP_LCD_WriteCmd(uint16_t cmd)
{
	BSP_LCD_CMD = cmd;
}




/*
*********************************************************************************************************
*                                         BSP_LCD_FSMC_Init()
*
* Description :  初始化LCD的FSMC
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_LCD_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static  void         BSP_LCD_FSMC_Init(void)
{
	OS_ERR                         err;
	FSMC_NORSRAMInitTypeDef        lcdInit;
	FSMC_NORSRAMTimingInitTypeDef  lcdTimigWr, lcdTimigRdWr;
	
	
	// 读写时序配置   很奇怪在F407上emwin使用外部SRAM时，该时间太短就会导致读点不正确比如
	// FSMC_DataSetupTime=04，FSMC_AddressSetupTime=03 而使用内部SRAM就不会出现该错误
	lcdTimigRdWr.FSMC_AccessMode            = FSMC_AccessMode_A;                //模式A参见参考手册
	lcdTimigRdWr.FSMC_DataSetupTime         = 0x05;                             // x/55ns = 168/1000 ns ,x = 9.24
	lcdTimigRdWr.FSMC_AddressSetupTime      = 0x04;                             //地址建立时间
	lcdTimigRdWr.FSMC_CLKDivision           = 0x00;                             //LCD工作在异步模式该位无意义                      
	lcdTimigRdWr.FSMC_DataLatency           = 0x00;                             //表示数据延迟周期，LCD工作在异步模式该位无意义 
	lcdTimigRdWr.FSMC_AddressHoldTime       = 0x00;                             //使用与模式D,模式A该位无意义
	lcdTimigRdWr.FSMC_BusTurnAroundDuration = 0x01;                             //LCD 该位无意义
	
	// 写时序配置   很奇怪在F407上emwin使用外部SRAM时，该时间太长就会导致读点不正确比如
	// FSMC_DataSetupTime=04，FSMC_AddressSetupTime=05 而使用内部SRAM就不会出现该错误
	
	lcdTimigWr.FSMC_AccessMode            = FSMC_AccessMode_A;                //模式A参见参考手册
	lcdTimigWr.FSMC_DataSetupTime         = 0x03;                             // x/55ns = 168/1000 ns ,x = 9.24
	lcdTimigWr.FSMC_AddressSetupTime      = 0x02;                             //地址建立时间
	lcdTimigWr.FSMC_CLKDivision           = 0x00;                             //LCD工作在异步模式该位无意义                      
	lcdTimigWr.FSMC_DataLatency           = 0x00;                             //表示数据延迟周期，LCD工作在异步模式该位无意义 
	lcdTimigWr.FSMC_AddressHoldTime       = 0x00;                             //适用与模式D,模式A该位无意义
	lcdTimigWr.FSMC_BusTurnAroundDuration = 0x01;                             //LCD 该位无意义

	lcdInit.FSMC_MemoryType            = FSMC_MemoryType_SRAM;                //存储器类型
	lcdInit.FSMC_Bank                  = BSP_LCD_Bank;                        //LCD所在块
	lcdInit.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;         //不复用地址引脚与数据引脚
	lcdInit.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;          //允许写访问
	lcdInit.FSMC_ExtendedMode          = FSMC_ExtendedMode_Enable;            //读写时序可以分别配置
	lcdInit.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;            //外部存储器数据宽度
	
	lcdInit.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;        //仅适用于同步存储器
	lcdInit.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;       //该LCD无等待引脚
	lcdInit.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;         //等待信号为高电平有效，LCD无用
	lcdInit.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;             //用于NOR
	lcdInit.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	lcdInit.FSMC_WrapMode              = FSMC_WrapMode_Disable;               //仅在突发模式下有效
	lcdInit.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;             //仅在同步模式下有效
	
	lcdInit.FSMC_ReadWriteTimingStruct = &lcdTimigRdWr;
	lcdInit.FSMC_WriteTimingStruct     = &lcdTimigWr;

	FSMC_NORSRAMInit(&lcdInit);
	FSMC_NORSRAMCmd(BSP_LCD_Bank, ENABLE);


	// 等待FSMC稳定
	
	OSTimeDlyHMSM( 0, 0, 0, 10,
			       OS_OPT_TIME_HMSM_STRICT,
			       &err );
}





/*
*********************************************************************************************************
*                                         BSP_LCD_CTRL_GPIO_Init()
*
* Description :  初始化LCD的片选引脚与背光引脚
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_LCD_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/


static  void    BSP_LCD_CTRL_GPIO_Init (void)
{
	GPIO_InitTypeDef gpioInit;
	
	// 开启控制引脚端口时钟
	
	RCC_AHB1PeriphClockCmd(LCD_GPIO_PORT_RCC_NCE, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_GPIO_PORT_RCC_BL, ENABLE);
	
	// 复用片选控制引脚
	
	GPIO_PinAFConfig(LCD_GPIO_PORT_NCE, LCD_PIN_SOURCE_NCE, GPIO_AF_FSMC);
	
	// 配置控制引脚模式
	
	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// 初始化片选
	
	gpioInit.GPIO_Pin   = LCD_PIN_NCE;                                        
	GPIO_Init(LCD_GPIO_PORT_NCE, &gpioInit);
	
	// 初始化背光引脚
	
	gpioInit.GPIO_Pin   = LCD_PIN_BL; 
	gpioInit.GPIO_Mode  = GPIO_Mode_OUT;                               
	GPIO_Init(LCD_GPIO_PORT_BL, &gpioInit);
}


/*
*********************************************************************************************************
*                                        BSP_LCD_DMA_Config()
*
* Description : 配置DMA，将DMA链接到内存内存
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_LCD_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if  LCD_DMA_EN > 0

static void BSP_LCD_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(LCD_DMA_CLK, ENABLE);      			                     //开启DMA时钟
	
	//复位DMA
	DMA_DeInit(LCD_DMA_STREAM);
	while (DMA_GetCmdStatus(LCD_DMA_STREAM) != DISABLE) {
	}
	
	DMA_InitStructure.DMA_Channel = LCD_DMA_CHANNEL;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;    			             //关闭FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                      //单数据发送
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                                    //循环模式才能开启双缓冲模式
	
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;      //两个字节传输
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                              //配置优先级
	
	DMA_InitStructure.DMA_BufferSize = 0;            		                         //目标地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BSP_LCD_DAT_BASE;              //源地址，只能是片内SRAM
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)BSP_LCD_DAT_BASE;   
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;                         //内存地址增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                  
	
	
	DMA_Init(LCD_DMA_STREAM, &DMA_InitStructure);
}

#endif  /* LCD_DMA_EN */



/*
*********************************************************************************************************
*                                         BSP_LCD_OpenWindow()
*
* Description :  开窗函数
*
* Argument(s) :  usX       x的起始坐标
*                usY       y的起始坐标
*                usWidth   x轴的宽度
*                usHeight  y轴的长度
*
* Return(s)   :  none
*
* Caller(s)   :  Application
*
* Note(s)     :  none.
*********************************************************************************************************
*/
static void BSP_LCD_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{	
	// 设置X坐标 
	
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 				
	BSP_LCD_WriteDat ( usX >> 8  );	                         /* 先高8位，然后低8位 */
	BSP_LCD_WriteDat ( usX & 0xFF);	                         /* 设置起始点和结束点*/
	BSP_LCD_WriteDat ( ( usX + usWidth - 1 ) >> 8  );
	BSP_LCD_WriteDat ( ( usX + usWidth - 1 ) & 0xff  );

	// 设置Y坐标 
	
	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 			     
	BSP_LCD_WriteDat ( usY >> 8  );
	BSP_LCD_WriteDat ( usY & 0xff  );
	BSP_LCD_WriteDat ( ( usY + usHeight - 1 ) >> 8 );
	BSP_LCD_WriteDat ( ( usY + usHeight - 1) & 0xff );
}




/*
*********************************************************************************************************
*                                         ILI9341_REG_Config()
*
* Description :  初始化ILI9341芯片
*
* Argument(s) :  none
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_LCD_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static void ILI9341_REG_Config ( void )
{
	OS_ERR     err;
	
	/*  Power control B (CFh)  */
	
	BSP_LCD_WriteCmd ( 0xCF  );
	BSP_LCD_WriteDat ( 0x00  );
	BSP_LCD_WriteDat ( 0x81  );
	BSP_LCD_WriteDat ( 0x30  );
	
	/*  Power on sequence control (EDh) */

	BSP_LCD_WriteCmd ( 0xED );
	BSP_LCD_WriteDat ( 0x64 );
	BSP_LCD_WriteDat ( 0x03 );
	BSP_LCD_WriteDat ( 0x12 );
	BSP_LCD_WriteDat ( 0x81 );
	
	/*  Driver timing control A (E8h) */
	
	BSP_LCD_WriteCmd ( 0xE8 );
	BSP_LCD_WriteDat ( 0x85 );
	BSP_LCD_WriteDat ( 0x10 );
	BSP_LCD_WriteDat ( 0x78 );
	
	/*  Power control A (CBh) */
	
	BSP_LCD_WriteCmd ( 0xCB );
	BSP_LCD_WriteDat ( 0x39 );
	BSP_LCD_WriteDat ( 0x2C );
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ( 0x34 );
	BSP_LCD_WriteDat ( 0x02 );
	
	/* Pump ratio control (F7h) */
	
	BSP_LCD_WriteCmd ( 0xF7 );
	BSP_LCD_WriteDat ( 0x20 );
	
	/* Driver timing control B */

	BSP_LCD_WriteCmd ( 0xEA );
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ( 0x00 );
	
	/* Frame Rate Control (In Normal Mode/Full Colors) (B1h) */
	
	BSP_LCD_WriteCmd ( 0xB1 );
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ( 0x1B );
	
	/*  Display Function Control (B6h) */
	
	BSP_LCD_WriteCmd ( 0xB6 );
	BSP_LCD_WriteDat ( 0x0A );
	BSP_LCD_WriteDat ( 0xA2 );
	
	/* Power Control 1 (C0h) */

	BSP_LCD_WriteCmd ( 0xC0 );
	BSP_LCD_WriteDat ( 0x35 );
	
	/* Power Control 2 (C1h) */
	
	BSP_LCD_WriteCmd ( 0xC1 );
	BSP_LCD_WriteDat ( 0x11 );
	
	/* VCOM Control 1 (C5h) */
	BSP_LCD_WriteCmd ( 0xC5 );
	BSP_LCD_WriteDat ( 0x45 );
	BSP_LCD_WriteDat ( 0x45 );
	
	/*  VCOM Control 2 (C7h)  */
	BSP_LCD_WriteCmd ( 0xC7 );
	BSP_LCD_WriteDat ( 0xA2 );
	
	/* Enable 3G (F2h) */
	BSP_LCD_WriteCmd ( 0xF2 );
	BSP_LCD_WriteDat ( 0x00 );
	
	/* Gamma Set (26h) */
	BSP_LCD_WriteCmd ( 0x26 );
	BSP_LCD_WriteDat ( 0x01 );
	
	
	/* Positive Gamma Correction */
	BSP_LCD_WriteCmd ( 0xE0 ); //Set Gamma
	BSP_LCD_WriteDat ( 0x0F );
	BSP_LCD_WriteDat ( 0x26 );
	BSP_LCD_WriteDat ( 0x24 );
	BSP_LCD_WriteDat ( 0x0B );
	BSP_LCD_WriteDat ( 0x0E );
	BSP_LCD_WriteDat ( 0x09 );
	BSP_LCD_WriteDat ( 0x54 );
	BSP_LCD_WriteDat ( 0xA8 );
	BSP_LCD_WriteDat ( 0x46 );
	BSP_LCD_WriteDat ( 0x0C );
	BSP_LCD_WriteDat ( 0x17 );
	BSP_LCD_WriteDat ( 0x09 );
	BSP_LCD_WriteDat ( 0x0F );
	BSP_LCD_WriteDat ( 0x07 );
	BSP_LCD_WriteDat ( 0x00 );
	
	/* Negative Gamma Correction (E1h) */
	BSP_LCD_WriteCmd ( 0XE1 ); //Set Gamma
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ( 0x19 );
	BSP_LCD_WriteDat ( 0x1B );
	BSP_LCD_WriteDat ( 0x04 );
	BSP_LCD_WriteDat ( 0x10 );
	BSP_LCD_WriteDat ( 0x07 );
	BSP_LCD_WriteDat ( 0x2A );
	BSP_LCD_WriteDat ( 0x47 );
	BSP_LCD_WriteDat ( 0x39 );
	BSP_LCD_WriteDat ( 0x03 );
	BSP_LCD_WriteDat ( 0x06 );
	BSP_LCD_WriteDat ( 0x06 );
	BSP_LCD_WriteDat ( 0x30 );
	BSP_LCD_WriteDat ( 0x38 );
	BSP_LCD_WriteDat ( 0x0F );
	
	/* memory access control set */
	
	BSP_LCD_WriteCmd ( 0x36 ); 	
	BSP_LCD_WriteDat ( 0xC8 );    /*竖屏  左上角到 (起点)到右下角 (终点)扫描方式*/

	
	/* column address control set */
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ((ILI9341_LESS_PIXEL-1)>>8);
	BSP_LCD_WriteDat ((ILI9341_LESS_PIXEL-1)&0xFF);
	
	/* page address control set */

	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ( 0x00 );
	BSP_LCD_WriteDat ((ILI9341_MORE_PIXEL-1)>>8 );
	BSP_LCD_WriteDat ((ILI9341_MORE_PIXEL-1)&0xFF);
	
	/*  Pixel Format Set (3Ah)  */

	BSP_LCD_WriteCmd ( 0x3a ); 
	BSP_LCD_WriteDat ( 0x55 );
	
	/* Sleep Out (11h)  */
	BSP_LCD_WriteCmd ( 0x11 );	
	
	OSTimeDlyHMSM( 0, 0, 0, 120,
		           OS_OPT_TIME_HMSM_STRICT,
                   &err );	
	
	/* Display ON (29h) */
	BSP_LCD_WriteCmd ( 0x29 ); 
}





/*
*********************************************************************************************************
*                                         ILI9341_GramScan()
*
* Description :  设置ILI9341的GRAM的扫描方向 
*
* Argument(s) :  ucOption : 设置GRAM扫描方向范围0-7，具体模式参见Doc文件夹下的说明
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_LCD_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static void ILI9341_GramScan (uint8_t ucOption)
{	
	//参数检查，只可输入0-7
	if(ucOption >7 )
		return;
	
	//根据模式更新LCD_SCAN_MODE的值，主要用于触摸屏选择计算参数
	LCD_SCAN_MODE = ucOption;
	
	//根据模式更新XY方向的像素宽度
	if(ucOption%2 == 0)	
	{
		// 0 2 4 6模式下X方向像素宽度为240，Y方向为320
		LCD_X_LENGTH = ILI9341_LESS_PIXEL;
		LCD_Y_LENGTH =	ILI9341_MORE_PIXEL;
	}
	else				
	{
		// 1 3 5 7模式下X方向像素宽度为320，Y方向为240
		LCD_X_LENGTH = ILI9341_MORE_PIXEL;
		LCD_Y_LENGTH =	ILI9341_LESS_PIXEL; 
	}

	// 0x36命令参数的高3位可用于设置GRAM扫描方向	
	
	BSP_LCD_WriteCmd ( 0x36 ); 
	BSP_LCD_WriteDat ( 0x08 |(ucOption<<5));            //根据ucOption的值设置LCD参数，共0-7种模式
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 
	BSP_LCD_WriteDat ( 0x00 );		                    // x 起始坐标高8位 
	BSP_LCD_WriteDat ( 0x00 );		                    // x 起始坐标低8位 
	BSP_LCD_WriteDat ( ((LCD_X_LENGTH-1)>>8)&0xFF );    // x 结束坐标高8位 	
	BSP_LCD_WriteDat ( (LCD_X_LENGTH-1)&0xFF );		    // x 结束坐标低8位 

	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 
	BSP_LCD_WriteDat ( 0x00 );		                    // y 起始坐标高8位 
	BSP_LCD_WriteDat ( 0x00 );		                    // y 起始坐标低8位 
	BSP_LCD_WriteDat ( ((LCD_Y_LENGTH-1)>>8)&0xFF );	// y 结束坐标高8位 	 
	BSP_LCD_WriteDat ( (LCD_Y_LENGTH-1)&0xFF );		    // y 结束坐标低8位 

	/* write gram start */
	BSP_LCD_WriteCmd ( CMD_SetPixel );	
}





/*
*********************************************************************************************************
*                                         BSP_LCD_SetCursor()
*
* Description :  设定ILI9341的光标坐标
*
* Argument(s) :  usX ：在特定扫描方向下光标的X坐标
*                usY ：在特定扫描方向下光标的Y坐标
*
* Return(s)   :  none
*
* Caller(s)   :  BSP_LCD_Init()
*
* Note(s)     :  none.
*********************************************************************************************************
*/

static void BSP_LCD_SetCursor ( uint16_t usX, uint16_t usY )	
{
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 				 /* 设置X坐标 */
	BSP_LCD_WriteDat ( usX >> 8  );	                         /* 先高8位，然后低8位 */
	BSP_LCD_WriteDat ( usX & 0xff  );	                     /* 设置起始点和结束点*/
	BSP_LCD_WriteDat ( usX >> 8  );	                         /* 先高8位，然后低8位 */
	BSP_LCD_WriteDat ( usX & 0xff  );	                     /* 设置起始点和结束点*/

	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 			     /* 设置Y坐标*/
	BSP_LCD_WriteDat ( usY >> 8  );                            
	BSP_LCD_WriteDat ( usY & 0xff  );
	BSP_LCD_WriteDat ( usY >> 8  );
	BSP_LCD_WriteDat ( usY & 0xff  );
}




