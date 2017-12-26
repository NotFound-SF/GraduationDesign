


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_touch.h"


/*
*********************************************************************************************************
*                                             滤波相关的宏
*********************************************************************************************************
*/
#define   SAMP_CNT             4
#define   SAMP_CNT_DIV2        2


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void                   XPT2046_GPIO_SPI_Config               ( void );

static void                   XPT2046_DelayUS                       ( __IO uint32_t ulCount );
static void                   XPT2046_WriteCMD                      ( uint8_t ucCmd );
static uint16_t               XPT2046_ReadCMD                       ( void );




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        XPT2046_Init()
*
* Description : 初始化XPT2046触摸控制芯片的对应引脚
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

void XPT2046_Init ( void )
{
	XPT2046_GPIO_SPI_Config ();		
}



/*
*********************************************************************************************************
*                                        XPT2046_ReadAdc_Fliter()
*
* Description : 选择一个模拟通道，启动ADC，并返回ADC采样结果
*
* Argument(s) : ucCh = 0x90 表示Y通道； 0xd0 表示X通道
*
* Return(s)   : 滤波后的12位ADC值
*
* Caller(s)   : Applacation
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint16_t XPT2046_ReadAdc_Fliter(uint8_t channel)
{
  u8 i, j, min;
  u16 temp;
  u16 tempXY[SAMP_CNT];
  
  static uint16_t adc_x = 0,adc_y = 0;

  if(TOUCH_PEN_RD()== macXPT2046_INT_ActiveLevel) 
  {
    for(i = 0;i < SAMP_CNT;i++)
    {
      XPT2046_WriteCMD(channel);
      tempXY[i] = XPT2046_ReadCMD();     
    }
    // 降序排列
    for(i=0; i<SAMP_CNT-1; i++)
    {
      min=i;
      for (j=i+1; j<SAMP_CNT; j++)
      {
        if (tempXY[min] > tempXY[j]) min=j;
      }
      temp = tempXY[i];
      tempXY[i] = tempXY[min];
      tempXY[min] = temp;
    }
   
    // 设定阈值
    if((tempXY[SAMP_CNT_DIV2]-tempXY[SAMP_CNT_DIV2-1]) > 5)
    {
      /* 若两个中间值相差太远，则舍弃这个新数据，返回上一次的触摸数据*/
      if(channel == macXPT2046_CHANNEL_Y )
        return adc_x; //x通道
      else
        return adc_y; //y通道
    
    }
    
   // 求中间值的均值   
   if(channel == macXPT2046_CHANNEL_Y)
   {
     adc_x = (tempXY[SAMP_CNT_DIV2]+tempXY[SAMP_CNT_DIV2-1]) / 2;
      return adc_x;
    }      
    else
    {
      adc_y = (tempXY[SAMP_CNT_DIV2]+tempXY[SAMP_CNT_DIV2-1]) / 2;
      return adc_y;
    }
       
  }
  else
  {
    return 0;   //没有触摸，返回0
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
*                                        XPT2046_GPIO_SPI_Config()
*
* Description : 初始化XPT2046触摸控制芯片的对应引脚,配置成模拟SPI相应的模式
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : XPT2046_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void XPT2046_GPIO_SPI_Config ( void ) 
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;
	

    // 开启GPIO时钟 
	
	RCC_AHB1PeriphClockCmd(TOUCH_PEN_GPIO_CLK, ENABLE);             // 开启中断引脚时钟
	RCC_AHB1PeriphClockCmd(TOUCH_CS_GPIO_CLK, ENABLE);              // 开启片选引脚时钟
	RCC_AHB1PeriphClockCmd(TOUCH_CLK_GPIO_CLK, ENABLE);             // 开启模拟时钟引脚时钟
	RCC_AHB1PeriphClockCmd(TOUCH_MOSI_GPIO_CLK, ENABLE);            // 开启MOSI引脚时钟
	RCC_AHB1PeriphClockCmd(TOUCH_MISO_GPIO_CLK, ENABLE);            // 开启MISO引脚时钟
	
 
	// 初始化 CS, MOSI, CLK
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_CS_PIN;                    // CS引脚
	GPIO_Init(TOUCH_CS_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_MOSI_PIN;                  // MOSI引脚
	GPIO_Init(TOUCH_MOSI_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_CLK_PIN;                   // CLK引脚
	GPIO_Init(TOUCH_CLK_PORT, &GPIO_InitStructure);
	
	
	// 初始化 MISO, PEN
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = TOUCH_MISO_PIN;                   // MISO引脚
	GPIO_Init(TOUCH_MISO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = TOUCH_PEN_PIN;                    // PEN引脚
	GPIO_Init(TOUCH_PEN_PORT, &GPIO_InitStructure);
	
  
    // 拉低片选，选择XPT2046 
	TOUCH_CS_ENABLE();
}







/*
*********************************************************************************************************
*                                        XPT2046_DelayUS()
*
* Description : 模拟SPI所需要的延时，虽然是多任务系统但是因为存在时钟信号，所以不怕被别的进程打断
*
* Argument(s) : ulCount 要延时的微妙数
*
* Return(s)   : none
*
* Caller(s)   : XPT2046_WriteCMD()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void XPT2046_DelayUS ( __IO uint32_t ulCount )
{
	uint32_t i;


	for ( i = 0; i < ulCount; i ++ )
	{
		uint8_t uc = 28;     //设置值为12，大约延1微秒  
	      
		while ( uc -- );     //延1微秒	

	}
	
}



/*
*********************************************************************************************************
*                                        XPT2046_WriteCMD()
*
* Description : SPI写函数
*
* Argument(s) : ucCmd 要写入的指令
*
* Return(s)   : none
*
* Caller(s)   : XPT2046_ReadAdc_Fliter()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void XPT2046_WriteCMD ( uint8_t ucCmd ) 
{
	uint8_t i;


	TOUCH_MOSI_0();
	
	TOUCH_CLK_LOW();

	for ( i = 0; i < 8; i ++ ) 
	{
		
		if (( ucCmd >> ( 7 - i ) ) & 0x01) {
			TOUCH_MOSI_1();
		} else {
			TOUCH_MOSI_0();
		}
		
	  XPT2046_DelayUS ( 5 );
		
		TOUCH_CLK_HIGH();

	  XPT2046_DelayUS ( 5 );

		TOUCH_CLK_LOW();
	}
	
}


/*
*********************************************************************************************************
*                                        XPT2046_ReadCMD()
*
* Description : SPI读函数
*
* Argument(s) : none
*
* Return(s)   : 读取到的数据
*
* Caller(s)   : XPT2046_ReadAdc_Fliter()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static uint16_t XPT2046_ReadCMD ( void ) 
{
	uint8_t i;
	uint16_t usBuf=0, usTemp;
	


	TOUCH_MOSI_0();

	TOUCH_CLK_HIGH();

	for ( i=0;i<12;i++ ) 
	{
		TOUCH_CLK_LOW();    
	
		usTemp = TOUCH_MISO_RD();
		
		usBuf |= usTemp << ( 11 - i );
	
		TOUCH_CLK_HIGH();
		
	}
	
	return usBuf;

}



