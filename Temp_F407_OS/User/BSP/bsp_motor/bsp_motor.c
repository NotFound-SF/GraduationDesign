


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_motor.h"


/*
*********************************************************************************************************
*                                           LOCAL DATA
*********************************************************************************************************
*/

static  OS_SEM    SemLock;                                        //确保只有一个进程调用步进电机,否则会干扰步进电机转动

static  const uint8_t BeatCode[8] = {                             //步进电机节拍对应的IO控制代码
     0xE, 0xC, 0xD, 0x9, 0xB, 0x3, 0x7, 0x6
};


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void            BSP_MOTOR_GPIO_Init(void);
static  void            BSP_MOTOR_GPIO_OUT(uint8_t dat);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        BSP_MOTOR_Init()
*
* Description : 初始化步进电机
*
* Argument(s) : none
*
* Return(s)   : DEF_FAIL
*             ：DEF_OK
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_MOTOR_Init(void)
{
	CPU_ERR  err;
	
	// 创建锁定信号量
	
	OSSemCreate((OS_SEM    *)& SemLock,
                (CPU_CHAR  *)  "I2C SemLock", 
	            (OS_SEM_CTR )  1,
	            (OS_ERR    *)  &err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	BSP_MOTOR_GPIO_Init();
	BSP_MOTOR_GPIO_OUT(0x00);                        // 确保步进电机不通电          
	
	return DEF_OK;
}


/*
*********************************************************************************************************
*                                        BSP_Turn_Motor()
*
* Description : 以一定方向和角度转动进电机
*
* Argument(s) : angle   转动的角度
*               dir     MOTOR_DIR_RIGHT 面对转轴右转 
*                       MOTOR_DIR_RIGHT 面对转轴左转 
*
* Return(s)   : none    
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_Turn_Motor(uint16_t angle, MOTOR_DIR_X dir)
{
	CPU_ERR  err;
	uint8_t  index = 0;                                          //节拍输出索引
	uint32_t beats = (angle * 4076) / 360;                       //节拍数

	if(0 == angle) {
		return;
	}

	//阻塞永久等待，直到其他线程释放步进电机
	 
	OSSemPend((OS_SEM *)& SemLock,                               //锁定该端口
			  (OS_TICK )  0,
			  (OS_OPT  )  OS_OPT_PEND_BLOCKING,
			  (CPU_TS *)  0,
			  (OS_ERR *)& err);


	for ( ; beats>0; beats--) {
		
		if (MOTOR_DIR_RIGHT == dir) {
			index++; 
		} else {
			index--;
		}
		
		index = index & 0x07;                                    // 归零操作
		
		BSP_MOTOR_GPIO_OUT( BeatCode[index] );
		
		// 每个节拍延时2ms
		OSTimeDly((OS_TICK )  2, 
				  (OS_OPT  )  OS_OPT_TIME_DLY, 
				  (OS_ERR *)& err);
	}

	BSP_MOTOR_GPIO_OUT(0x00);                                   // 确保步进电机不通电   
	
	//转动结束释放信号量

	OSSemPost((OS_SEM *)& SemLock,
			  (OS_OPT  )  OS_OPT_POST_1,
			  (OS_ERR *)& err);
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
*                                        BSP_MOTOR_GPIO_Init()
*
* Description : 初始化 步进电机的GPIO引脚
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_MOTOR_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_MOTOR_GPIO_Init (void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
    // 开启GPIO时钟 
	
	RCC_AHB1PeriphClockCmd(MOTOR_IN1_GPIO_CLK, ENABLE);            // 开启IN1引脚时钟
	RCC_AHB1PeriphClockCmd(MOTOR_IN2_GPIO_CLK, ENABLE);            // 开启IN2引脚时钟
	RCC_AHB1PeriphClockCmd(MOTOR_IN3_GPIO_CLK, ENABLE);            // 开启IN3引脚时钟
	RCC_AHB1PeriphClockCmd(MOTOR_IN4_GPIO_CLK, ENABLE);            // 开启IN4引脚时钟
 
	
	// 初始化 IN1， IN2， IN3， IN4
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	
	GPIO_InitStructure.GPIO_Pin = MOTOR_IN1_PIN;                    // CS引脚
	GPIO_Init(MOTOR_IN1_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = MOTOR_IN2_PIN;                    // CS引脚
	GPIO_Init(MOTOR_IN2_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MOTOR_IN3_PIN;                    // CS引脚
	GPIO_Init(MOTOR_IN3_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MOTOR_IN4_PIN;                    // CS引脚
	GPIO_Init(MOTOR_IN4_PORT, &GPIO_InitStructure);
}




/*
*********************************************************************************************************
*                                        BSP_MOTOR_GPIO_OUT()
*
* Description : 使得驱动步进电机的四个脚输出相应的电平
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_Turn_Motor()
*
* Note(s)     : none.
*********************************************************************************************************
*/


static  void   BSP_MOTOR_GPIO_OUT(uint8_t dat)
{
	if (dat & DEF_BIT_00) 
		MOTOR_IN1_HIGH();
	else
		MOTOR_IN1_LOW();
	
	if (dat & DEF_BIT_01) 
		MOTOR_IN2_HIGH();
	else
		MOTOR_IN2_LOW();
	
	if (dat & DEF_BIT_02) 
		MOTOR_IN3_HIGH();
	else
		MOTOR_IN3_LOW();
	
	if (dat & DEF_BIT_03) 
		MOTOR_IN4_HIGH();
	else
		MOTOR_IN4_LOW();
		
}	



