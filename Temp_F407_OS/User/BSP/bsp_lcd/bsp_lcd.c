
/*
*********************************************************************************************************
*      因为接触的LCD较少所以该驱动只适用与ILI9341驱动的LCD并且必须是8080接口
*********************************************************************************************************
*/

#include "bsp_fsmc.h"
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


static  void         BSP_LCD_CTRL_GPIO_Init (void);
static  void         ILI9341_REG_Config     (void);
static  void         ILI9341_GramScan       (uint8_t ucOption);

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
	
	FSMC_NORSRAMInitTypeDef        lcdInit;
	FSMC_NORSRAMTimingInitTypeDef  lcdTimigWr, lcdTimigRd;
	
	// 初始化FSMC与初始化控制引脚
	
	BSP_FSMC_COMMON_Init();
	BSP_LCD_CTRL_GPIO_Init();
	
	
	// 读时序配置
	
	lcdTimigRd.FSMC_DataSetupTime         = 0x01;                             // x/55ns = 168/1000 ns ,x = 9.24
	lcdTimigRd.FSMC_AddressSetupTime      = 0x02;                             //地址建立时间
	
	lcdTimigRd.FSMC_AccessMode            = FSMC_AccessMode_B;                //模式A参见参考手册
//	lcdTimigRd.FSMC_DataSetupTime         = 0x05;                             // x/55ns = 168/1000 ns ,x = 9.24
//	lcdTimigRd.FSMC_AddressSetupTime      = 0x05;                             //地址建立时间
	lcdTimigRd.FSMC_CLKDivision           = 0x01;                             //LCD工作在异步模式该位无意义                      
	lcdTimigRd.FSMC_DataLatency           = 0x01;                             //表示数据延迟周期，LCD工作在异步模式该位无意义 
	lcdTimigRd.FSMC_AddressHoldTime       = 0x01;                             //使用与模式D,模式A该位无意义
	lcdTimigRd.FSMC_BusTurnAroundDuration = 0x00;                             //LCD 该位无意义
	
	// 写时序配置
	lcdTimigRd.FSMC_DataSetupTime         = 0x01;                             // x/55ns = 168/1000 ns ,x = 9.24
	lcdTimigRd.FSMC_AddressSetupTime      = 0x02;                             //地址建立时间
	
	lcdTimigWr.FSMC_AccessMode            = FSMC_AccessMode_B;                //模式A参见参考手册
//	lcdTimigWr.FSMC_DataSetupTime         = 0x05;                             // x/55ns = 168/1000 ns ,x = 9.24
//	lcdTimigWr.FSMC_AddressSetupTime      = 0x05;                             //地址建立时间
	lcdTimigWr.FSMC_CLKDivision           = 0x01;                             //LCD工作在异步模式该位无意义                      
	lcdTimigWr.FSMC_DataLatency           = 0x01;                             //表示数据延迟周期，LCD工作在异步模式该位无意义 
	lcdTimigWr.FSMC_AddressHoldTime       = 0x01;                             //适用与模式D,模式A该位无意义
	lcdTimigWr.FSMC_BusTurnAroundDuration = 0x00;                             //LCD 该位无意义

	lcdInit.FSMC_MemoryType            = FSMC_MemoryType_NOR;                 //存储器类型
	lcdInit.FSMC_Bank                  = BSP_LCD_Bank;                        //LCD所在块
	lcdInit.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;         //不复用地址引脚与数据引脚
	lcdInit.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;          //允许写访问
	lcdInit.FSMC_ExtendedMode          = FSMC_ExtendedMode_Enable;            //读写时序可以分别配置
	lcdInit.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;            //外部存储器数据宽度
	
	lcdInit.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;        //仅适用于同步存储器
	lcdInit.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;       //该LCD无等待引脚
	lcdInit.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_High;        //等待信号为高电平有效，LCD无用
	lcdInit.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;             //用于NOR
	lcdInit.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	lcdInit.FSMC_WrapMode              = FSMC_WrapMode_Disable;               //仅在突发模式下有效
	lcdInit.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;             //仅在同步模式下有效
	
	lcdInit.FSMC_ReadWriteTimingStruct = &lcdTimigWr;
	lcdInit.FSMC_WriteTimingStruct     = &lcdTimigRd;

	FSMC_NORSRAMInit(&lcdInit);
	FSMC_NORSRAMCmd(BSP_LCD_Bank, ENABLE);
	
	
	// 初始化LCD控制器
	
	ILI9341_REG_Config();
	// 配置DMA
#if  LCD_DMA_EN > 0	
	BSP_LCD_DMA_Config();
	
#endif /* LCD_DMA_EN */
	
	BSP_LCD_BL_ON();
	
	ILI9341_GramScan(LCD_SCAN_MODE);
}

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

__inline void BSP_LCD_WriteDat(uint16_t dat)
{
	BSP_LCD_DAT = dat;
}


/*
*********************************************************************************************************
*                                         BSP_LCD_ReadDat()
*
* Description :  从LCD读取数据
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

__inline uint16_t BSP_LCD_ReadDat(void)
{
	return BSP_LCD_DAT;
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

__inline void BSP_LCD_WriteCmd(uint16_t cmd)
{
	BSP_LCD_CMD = cmd;
}


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
void BSP_LCD_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
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







/**
 * @brief  读取ILI9341 GRAN 的一个像素数据
 * @param  无
 * @retval 像素数据
 */
static uint16_t ILI9341_Read_PixelData ( void )	
{	
	uint16_t usR=0, usG=0, usB=0 ;

	
	BSP_LCD_WriteCmd ( 0x2E );   /* 读数据 */
	
	usR = BSP_LCD_ReadDat (); 	/*FIRST READ OUT DUMMY DATA*/
	
	usR = BSP_LCD_ReadDat ();  	/*READ OUT RED DATA  */
	usB = BSP_LCD_ReadDat ();  	/*READ OUT BLUE DATA*/
	usG = BSP_LCD_ReadDat ();  	/*READ OUT GREEN DATA*/	
	
  return ( ( ( usR >> 11 ) << 11 ) | ( ( usG >> 10 ) << 5 ) | ( usB >> 11 ) );
	
}





uint16_t dat_l =  MAGENTA;



void LCD_Test(uint16_t *col)
{
	uint32_t i;
	uint16_t dat,temp;
	uint16_t usR=0, usG=0, usB=0 ;
	
	BSP_LCD_WriteCmd(CMD_SetPixel);
	
#if LCD_DMA_EN > 0
	
	// 注意每次传输未检测上次是否传输完成并且源地址不会增加
	LCD_DMA_STREAM->PAR   = (uint32_t)col;                    //数据源地址
	
	LCD_DMA_STREAM->NDTR = ILI9341_ALL_PIXEL/2;
	LCD_DMA_ID->LCD_IFCR |= LCD_DMA_CTC_MASK;
	LCD_DMA_STREAM->CR   |= DEF_BIT_00;                        //使能数据流
	
	while(0 == (LCD_DMA_ID->LCD_ISR & LCD_DMA_FTC_MASK));      //等待发送结束	
	
	LCD_DMA_STREAM->PAR   = (uint32_t)&dat_l;                  //数据源地址
	
	LCD_DMA_STREAM->NDTR = ILI9341_ALL_PIXEL/2;
	LCD_DMA_ID->LCD_IFCR |= LCD_DMA_CTC_MASK;
	LCD_DMA_STREAM->CR   |= DEF_BIT_00;                       //使能数据流
	
	while(0 == (LCD_DMA_ID->LCD_ISR & LCD_DMA_FTC_MASK));      //等待发送结束	

	
#else
	// 显然for循环需要多条指令，导致速度较慢
	for ( i = 0; i < ILI9341_ALL_PIXEL; i ++ )
		BSP_LCD_WriteDat(*col);
	
#endif /* LCD_DMA_EN */
	
	
	
	BSP_LCD_WriteCmd ( 0x2E );   /* 读数据 */
	usR = BSP_LCD_ReadDat (); 	/*FIRST READ OUT DUMMY DATA*/
		
	for (i = 0, dat=0; i < ILI9341_ALL_PIXEL; i++)
	{
		usR = BSP_LCD_ReadDat ();  	/*READ OUT RED DATA  */
		usB = BSP_LCD_ReadDat ();  	/*READ OUT BLUE DATA*/
		usG = BSP_LCD_ReadDat ();  	/*READ OUT GREEN DATA*/	
	
		dat = ( ( ( usR >> 11 ) << 11 ) | ( ( usG >> 10 ) << 5 ) | ( usB >> 11 ) );
		
		if (temp != dat) {
			temp = dat;
			BSP_UART_Printf(BSP_UART_ID_1, "i: %4X   dat: %4X   real1: %4X   real2: %4X\r\n", i, dat, *col, dat_l);
		}
		
		
		if (*col != dat && dat_l != dat) {
			BSP_UART_Printf(BSP_UART_ID_1, "error\r\n\r\n");
			BSP_UART_Printf(BSP_UART_ID_1, "i: %4X   dat: %4X   real1: %4X   real2: %4X\r\n", i, dat, *col, dat_l);
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
	
	OSTimeDlyHMSM( 0, 0, 0, 10,
		           OS_OPT_TIME_HMSM_STRICT,
                   &err );	
	
	/* Display ON (29h) */
	BSP_LCD_WriteCmd ( 0x29 ); 
}




/**
 * @brief  设置ILI9341的GRAM的扫描方向 
 * @param  ucOption ：选择GRAM的扫描方向 
 *     @arg 0-7 :参数可选值为0-7这八个方向
 *
 *	！！！其中0、3、5、6 模式适合从左至右显示文字，
 *				不推荐使用其它模式显示文字	其它模式显示文字会有镜像效果			
 *		
 *	其中0、2、4、6 模式的X方向像素为240，Y方向像素为320
 *	其中1、3、5、7 模式下X方向像素为320，Y方向像素为240
 *
 *	其中 6 模式为大部分液晶例程的默认显示方向
 *	其中 3 模式为摄像头例程使用的方向
 *	其中 0 模式为BMP图片显示例程使用的方向
 *
 * @retval 无
 * @note  坐标图例：A表示向上，V表示向下，<表示向左，>表示向右
					X表示X轴，Y表示Y轴

------------------------------------------------------------
模式0：				.		模式1：		.	模式2：			.	模式3：					
					A		.					A		.		A					.		A									
					|		.					|		.		|					.		|							
					Y		.					X		.		Y					.		X					
					0		.					1		.		2					.		3					
	<--- X0 o		.	<----Y1	o		.		o 2X--->  .		o 3Y--->	
------------------------------------------------------------	
模式4：				.	模式5：			.	模式6：			.	模式7：					
	<--- X4 o		.	<--- Y5 o		.		o 6X--->  .		o 7Y--->	
					4		.					5		.		6					.		7	
					Y		.					X		.		Y					.		X						
					|		.					|		.		|					.		|							
					V		.					V		.		V					.		V		
---------------------------------------------------------				
											 LCD屏示例
								|-----------------|
								|			秉火Logo		|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|-----------------|
								屏幕正面（宽240，高320）

 *******************************************************/
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
		//0 2 4 6模式下X方向像素宽度为240，Y方向为320
		LCD_X_LENGTH = ILI9341_LESS_PIXEL;
		LCD_Y_LENGTH =	ILI9341_MORE_PIXEL;
	}
	else				
	{
		//1 3 5 7模式下X方向像素宽度为320，Y方向为240
		LCD_X_LENGTH = ILI9341_MORE_PIXEL;
		LCD_Y_LENGTH =	ILI9341_LESS_PIXEL; 
	}

	//0x36命令参数的高3位可用于设置GRAM扫描方向	
	BSP_LCD_WriteCmd ( 0x36 ); 
	BSP_LCD_WriteDat ( 0x08 |(ucOption<<5));//根据ucOption的值设置LCD参数，共0-7种模式
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 
	BSP_LCD_WriteDat ( 0x00 );		/* x 起始坐标高8位 */
	BSP_LCD_WriteDat ( 0x00 );		/* x 起始坐标低8位 */
	BSP_LCD_WriteDat ( ((LCD_X_LENGTH-1)>>8)&0xFF ); /* x 结束坐标高8位 */	
	BSP_LCD_WriteDat ( (LCD_X_LENGTH-1)&0xFF );				/* x 结束坐标低8位 */

	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 
	BSP_LCD_WriteDat ( 0x00 );		/* y 起始坐标高8位 */
	BSP_LCD_WriteDat ( 0x00 );		/* y 起始坐标低8位 */
	BSP_LCD_WriteDat ( ((LCD_Y_LENGTH-1)>>8)&0xFF );	/* y 结束坐标高8位 */	 
	BSP_LCD_WriteDat ( (LCD_Y_LENGTH-1)&0xFF );				/* y 结束坐标低8位 */

	/* write gram start */
	BSP_LCD_WriteCmd ( CMD_SetPixel );	
}
