
/*
*********************************************************************************************************
*      ��Ϊ�Ӵ���LCD�������Ը�����ֻ������ILI9341������LCD���ұ�����8080�ӿ�
*********************************************************************************************************
*/

#include "bsp_fsmc.h"
#include "bsp_lcd.h"


//����Һ��ɨ�跽����仯��XY���ؿ��
//����ILI9341_GramScan�������÷���ʱ���Զ�����
uint16_t LCD_X_LENGTH = ILI9341_LESS_PIXEL;
uint16_t LCD_Y_LENGTH = ILI9341_MORE_PIXEL;


//Һ����ɨ��ģʽ����������Ҫ���ڷ���ѡ�������ļ������
//������ѡֵΪ0-7
//����ILI9341_GramScan�������÷���ʱ���Զ�����
//LCD�ճ�ʼ�����ʱ��ʹ�ñ�Ĭ��ֵ

uint8_t LCD_SCAN_MODE = 3;    //����
//uint8_t LCD_SCAN_MODE = 6;      //����


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
* Description :  ��ʼ��LCD��Ƭѡ�����뱳������
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
	
	// ��ʼ��FSMC���ʼ����������
	
	BSP_FSMC_COMMON_Init();
	BSP_LCD_CTRL_GPIO_Init();
	
	
	// ��ʱ������
	
	lcdTimigRd.FSMC_DataSetupTime         = 0x01;                             // x/55ns = 168/1000 ns ,x = 9.24
	lcdTimigRd.FSMC_AddressSetupTime      = 0x02;                             //��ַ����ʱ��
	
	lcdTimigRd.FSMC_AccessMode            = FSMC_AccessMode_B;                //ģʽA�μ��ο��ֲ�
//	lcdTimigRd.FSMC_DataSetupTime         = 0x05;                             // x/55ns = 168/1000 ns ,x = 9.24
//	lcdTimigRd.FSMC_AddressSetupTime      = 0x05;                             //��ַ����ʱ��
	lcdTimigRd.FSMC_CLKDivision           = 0x01;                             //LCD�������첽ģʽ��λ������                      
	lcdTimigRd.FSMC_DataLatency           = 0x01;                             //��ʾ�����ӳ����ڣ�LCD�������첽ģʽ��λ������ 
	lcdTimigRd.FSMC_AddressHoldTime       = 0x01;                             //ʹ����ģʽD,ģʽA��λ������
	lcdTimigRd.FSMC_BusTurnAroundDuration = 0x00;                             //LCD ��λ������
	
	// дʱ������
	lcdTimigRd.FSMC_DataSetupTime         = 0x01;                             // x/55ns = 168/1000 ns ,x = 9.24
	lcdTimigRd.FSMC_AddressSetupTime      = 0x02;                             //��ַ����ʱ��
	
	lcdTimigWr.FSMC_AccessMode            = FSMC_AccessMode_B;                //ģʽA�μ��ο��ֲ�
//	lcdTimigWr.FSMC_DataSetupTime         = 0x05;                             // x/55ns = 168/1000 ns ,x = 9.24
//	lcdTimigWr.FSMC_AddressSetupTime      = 0x05;                             //��ַ����ʱ��
	lcdTimigWr.FSMC_CLKDivision           = 0x01;                             //LCD�������첽ģʽ��λ������                      
	lcdTimigWr.FSMC_DataLatency           = 0x01;                             //��ʾ�����ӳ����ڣ�LCD�������첽ģʽ��λ������ 
	lcdTimigWr.FSMC_AddressHoldTime       = 0x01;                             //������ģʽD,ģʽA��λ������
	lcdTimigWr.FSMC_BusTurnAroundDuration = 0x00;                             //LCD ��λ������

	lcdInit.FSMC_MemoryType            = FSMC_MemoryType_NOR;                 //�洢������
	lcdInit.FSMC_Bank                  = BSP_LCD_Bank;                        //LCD���ڿ�
	lcdInit.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;         //�����õ�ַ��������������
	lcdInit.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;          //����д����
	lcdInit.FSMC_ExtendedMode          = FSMC_ExtendedMode_Enable;            //��дʱ����Էֱ�����
	lcdInit.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;            //�ⲿ�洢�����ݿ��
	
	lcdInit.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;        //��������ͬ���洢��
	lcdInit.FSMC_AsynchronousWait      = FSMC_AsynchronousWait_Disable;       //��LCD�޵ȴ�����
	lcdInit.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_High;        //�ȴ��ź�Ϊ�ߵ�ƽ��Ч��LCD����
	lcdInit.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;             //����NOR
	lcdInit.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
	lcdInit.FSMC_WrapMode              = FSMC_WrapMode_Disable;               //����ͻ��ģʽ����Ч
	lcdInit.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;             //����ͬ��ģʽ����Ч
	
	lcdInit.FSMC_ReadWriteTimingStruct = &lcdTimigWr;
	lcdInit.FSMC_WriteTimingStruct     = &lcdTimigRd;

	FSMC_NORSRAMInit(&lcdInit);
	FSMC_NORSRAMCmd(BSP_LCD_Bank, ENABLE);
	
	
	// ��ʼ��LCD������
	
	ILI9341_REG_Config();
	// ����DMA
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
* Description :  ���Դ�д������
*
* Argument(s) :  dat   Ҫд�������
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
* Description :  ��LCD��ȡ����
*
* Argument(s) :  dat   Ҫд�������
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
* Description :  ���Դ�д������
*
* Argument(s) :  cmd   Ҫд���ָ��
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
* Description :  ��������
*
* Argument(s) :  usX       x����ʼ����
*                usY       y����ʼ����
*                usWidth   x��Ŀ��
*                usHeight  y��ĳ���
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
	// ����X���� 
	
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 				
	BSP_LCD_WriteDat ( usX >> 8  );	                         /* �ȸ�8λ��Ȼ���8λ */
	BSP_LCD_WriteDat ( usX & 0xFF);	                         /* ������ʼ��ͽ�����*/
	BSP_LCD_WriteDat ( ( usX + usWidth - 1 ) >> 8  );
	BSP_LCD_WriteDat ( ( usX + usWidth - 1 ) & 0xff  );

	// ����Y���� 
	
	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 			     
	BSP_LCD_WriteDat ( usY >> 8  );
	BSP_LCD_WriteDat ( usY & 0xff  );
	BSP_LCD_WriteDat ( ( usY + usHeight - 1 ) >> 8 );
	BSP_LCD_WriteDat ( ( usY + usHeight - 1) & 0xff );
}







/**
 * @brief  ��ȡILI9341 GRAN ��һ����������
 * @param  ��
 * @retval ��������
 */
static uint16_t ILI9341_Read_PixelData ( void )	
{	
	uint16_t usR=0, usG=0, usB=0 ;

	
	BSP_LCD_WriteCmd ( 0x2E );   /* ������ */
	
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
	
	// ע��ÿ�δ���δ����ϴ��Ƿ�����ɲ���Դ��ַ��������
	LCD_DMA_STREAM->PAR   = (uint32_t)col;                    //����Դ��ַ
	
	LCD_DMA_STREAM->NDTR = ILI9341_ALL_PIXEL/2;
	LCD_DMA_ID->LCD_IFCR |= LCD_DMA_CTC_MASK;
	LCD_DMA_STREAM->CR   |= DEF_BIT_00;                        //ʹ��������
	
	while(0 == (LCD_DMA_ID->LCD_ISR & LCD_DMA_FTC_MASK));      //�ȴ����ͽ���	
	
	LCD_DMA_STREAM->PAR   = (uint32_t)&dat_l;                  //����Դ��ַ
	
	LCD_DMA_STREAM->NDTR = ILI9341_ALL_PIXEL/2;
	LCD_DMA_ID->LCD_IFCR |= LCD_DMA_CTC_MASK;
	LCD_DMA_STREAM->CR   |= DEF_BIT_00;                       //ʹ��������
	
	while(0 == (LCD_DMA_ID->LCD_ISR & LCD_DMA_FTC_MASK));      //�ȴ����ͽ���	

	
#else
	// ��Ȼforѭ����Ҫ����ָ������ٶȽ���
	for ( i = 0; i < ILI9341_ALL_PIXEL; i ++ )
		BSP_LCD_WriteDat(*col);
	
#endif /* LCD_DMA_EN */
	
	
	
	BSP_LCD_WriteCmd ( 0x2E );   /* ������ */
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
* Description :  ��ʼ��LCD��Ƭѡ�����뱳������
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
	
	// �����������Ŷ˿�ʱ��
	
	RCC_AHB1PeriphClockCmd(LCD_GPIO_PORT_RCC_NCE, ENABLE);
	RCC_AHB1PeriphClockCmd(LCD_GPIO_PORT_RCC_BL, ENABLE);
	
	// ����Ƭѡ��������
	
	GPIO_PinAFConfig(LCD_GPIO_PORT_NCE, LCD_PIN_SOURCE_NCE, GPIO_AF_FSMC);
	
	// ���ÿ�������ģʽ
	
	gpioInit.GPIO_Mode  = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	
	// ��ʼ��Ƭѡ
	
	gpioInit.GPIO_Pin   = LCD_PIN_NCE;                                        
	GPIO_Init(LCD_GPIO_PORT_NCE, &gpioInit);
	
	// ��ʼ����������
	
	gpioInit.GPIO_Pin   = LCD_PIN_BL; 
	gpioInit.GPIO_Mode  = GPIO_Mode_OUT;                               
	GPIO_Init(LCD_GPIO_PORT_BL, &gpioInit);
}


/*
*********************************************************************************************************
*                                        BSP_LCD_DMA_Config()
*
* Description : ����DMA����DMA���ӵ��ڴ��ڴ�
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
	
	RCC_AHB1PeriphClockCmd(LCD_DMA_CLK, ENABLE);      			                     //����DMAʱ��
	
	//��λDMA
	DMA_DeInit(LCD_DMA_STREAM);
	while (DMA_GetCmdStatus(LCD_DMA_STREAM) != DISABLE) {
	}
	
	DMA_InitStructure.DMA_Channel = LCD_DMA_CHANNEL;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;    			             //�ر�FIFO
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                      //�����ݷ���
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                                    //ѭ��ģʽ���ܿ���˫����ģʽ
	
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;      //�����ֽڴ���
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                              //�������ȼ�
	
	DMA_InitStructure.DMA_BufferSize = 0;            		                         //Ŀ���ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BSP_LCD_DAT_BASE;              //Դ��ַ��ֻ����Ƭ��SRAM
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)BSP_LCD_DAT_BASE;   
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;                         //�ڴ��ַ��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                  
	
	
	DMA_Init(LCD_DMA_STREAM, &DMA_InitStructure);
}

#endif  /* LCD_DMA_EN */




/*
*********************************************************************************************************
*                                         ILI9341_REG_Config()
*
* Description :  ��ʼ��ILI9341оƬ
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
	BSP_LCD_WriteDat ( 0xC8 );    /*����  ���Ͻǵ� (���)�����½� (�յ�)ɨ�跽ʽ*/

	
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
 * @brief  ����ILI9341��GRAM��ɨ�跽�� 
 * @param  ucOption ��ѡ��GRAM��ɨ�跽�� 
 *     @arg 0-7 :������ѡֵΪ0-7��˸�����
 *
 *	����������0��3��5��6 ģʽ�ʺϴ���������ʾ���֣�
 *				���Ƽ�ʹ������ģʽ��ʾ����	����ģʽ��ʾ���ֻ��о���Ч��			
 *		
 *	����0��2��4��6 ģʽ��X��������Ϊ240��Y��������Ϊ320
 *	����1��3��5��7 ģʽ��X��������Ϊ320��Y��������Ϊ240
 *
 *	���� 6 ģʽΪ�󲿷�Һ�����̵�Ĭ����ʾ����
 *	���� 3 ģʽΪ����ͷ����ʹ�õķ���
 *	���� 0 ģʽΪBMPͼƬ��ʾ����ʹ�õķ���
 *
 * @retval ��
 * @note  ����ͼ����A��ʾ���ϣ�V��ʾ���£�<��ʾ����>��ʾ����
					X��ʾX�ᣬY��ʾY��

------------------------------------------------------------
ģʽ0��				.		ģʽ1��		.	ģʽ2��			.	ģʽ3��					
					A		.					A		.		A					.		A									
					|		.					|		.		|					.		|							
					Y		.					X		.		Y					.		X					
					0		.					1		.		2					.		3					
	<--- X0 o		.	<----Y1	o		.		o 2X--->  .		o 3Y--->	
------------------------------------------------------------	
ģʽ4��				.	ģʽ5��			.	ģʽ6��			.	ģʽ7��					
	<--- X4 o		.	<--- Y5 o		.		o 6X--->  .		o 7Y--->	
					4		.					5		.		6					.		7	
					Y		.					X		.		Y					.		X						
					|		.					|		.		|					.		|							
					V		.					V		.		V					.		V		
---------------------------------------------------------				
											 LCD��ʾ��
								|-----------------|
								|			����Logo		|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|-----------------|
								��Ļ���棨��240����320��

 *******************************************************/
static void ILI9341_GramScan (uint8_t ucOption)
{	
	//������飬ֻ������0-7
	if(ucOption >7 )
		return;
	
	//����ģʽ����LCD_SCAN_MODE��ֵ����Ҫ���ڴ�����ѡ��������
	LCD_SCAN_MODE = ucOption;
	
	//����ģʽ����XY��������ؿ��
	if(ucOption%2 == 0)	
	{
		//0 2 4 6ģʽ��X�������ؿ��Ϊ240��Y����Ϊ320
		LCD_X_LENGTH = ILI9341_LESS_PIXEL;
		LCD_Y_LENGTH =	ILI9341_MORE_PIXEL;
	}
	else				
	{
		//1 3 5 7ģʽ��X�������ؿ��Ϊ320��Y����Ϊ240
		LCD_X_LENGTH = ILI9341_MORE_PIXEL;
		LCD_Y_LENGTH =	ILI9341_LESS_PIXEL; 
	}

	//0x36��������ĸ�3λ����������GRAMɨ�跽��	
	BSP_LCD_WriteCmd ( 0x36 ); 
	BSP_LCD_WriteDat ( 0x08 |(ucOption<<5));//����ucOption��ֵ����LCD��������0-7��ģʽ
	BSP_LCD_WriteCmd ( CMD_SetCoordinateX ); 
	BSP_LCD_WriteDat ( 0x00 );		/* x ��ʼ�����8λ */
	BSP_LCD_WriteDat ( 0x00 );		/* x ��ʼ�����8λ */
	BSP_LCD_WriteDat ( ((LCD_X_LENGTH-1)>>8)&0xFF ); /* x ���������8λ */	
	BSP_LCD_WriteDat ( (LCD_X_LENGTH-1)&0xFF );				/* x ���������8λ */

	BSP_LCD_WriteCmd ( CMD_SetCoordinateY ); 
	BSP_LCD_WriteDat ( 0x00 );		/* y ��ʼ�����8λ */
	BSP_LCD_WriteDat ( 0x00 );		/* y ��ʼ�����8λ */
	BSP_LCD_WriteDat ( ((LCD_Y_LENGTH-1)>>8)&0xFF );	/* y ���������8λ */	 
	BSP_LCD_WriteDat ( (LCD_Y_LENGTH-1)&0xFF );				/* y ���������8λ */

	/* write gram start */
	BSP_LCD_WriteCmd ( CMD_SetPixel );	
}
