
/*
*********************************************************************************************************
*              该文件用于精确阻塞延时，使用硬件定时器10作为时钟源,因为定时器10挂在APB2(84Mhz)总线，所以时钟
*              更精确目的是保证非标准接口时序的正确性,例如用于DS18B20为了保证延时的精确性，请确保进程的优先级
*              足够高，否则会有其他进程耗时；
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_timing.h"

/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

typedef struct{
	OS_SEM   SemLock;                                  //该IIC独占信号量                                
    OS_SEM   SemWait;                                  //IIC等待信号量     
}BSP_TIMING_STATUS;



/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  BSP_TIMING_STATUS      TimingStatus;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void         NVIC_Configuration   (void);


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        BSP_Timing_Init()
*
* Description : 初始延时函数
*
* Argument(s) : none
*
* Return(s)   : DEF_OK     初始化成功
*               DEF_FAIL   初始化失败
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_Timing_Init (void)
{
	OS_ERR err;
	TIM_TimeBaseInitTypeDef baseInit;
	
	TIMING_PERIPH_RCC_CMD(TIMING_PERIPH_RCC, ENABLE);                       //开启定时器RCC时钟
	                                                                        //创建独占信号量
	OSSemCreate((OS_SEM    *)&(TimingStatus.SemLock),                      
                (CPU_CHAR  *)"Timing SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	                                                                        //创建锁定信号量
	OSSemCreate((OS_SEM    *)&(TimingStatus.SemWait),                      
                (CPU_CHAR  *)"Timing SemWait", 
	            (OS_SEM_CTR ) 0,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
	baseInit.TIM_Period            = 0xFFFF;                                //计数周期
	baseInit.TIM_Prescaler         = 0xFFFF;                                //对84M时钟分频
	baseInit.TIM_CounterMode       = TIM_CounterMode_Up;                    //递增计数
	baseInit.TIM_ClockDivision     = TIM_CKD_DIV1;                          //对定时器10无用
	baseInit.TIM_RepetitionCounter = 0x00;                                  //只适用高级定时器1和8
	                               
	NVIC_Configuration();                                                   //连接到NVIC
	TIM_TimeBaseInit(TIMING_TYPE, &baseInit);                               //改语句必须在下一句前面否则一使能就会进入更新中断
	TIM_ClearITPendingBit(TIMING_TYPE, TIM_IT_Update);
	TIM_ITConfig(TIMING_TYPE, TIM_IT_Update, ENABLE);                       //开启上溢中断
	
	TIMING_TYPE -> CR1 |= DEF_BIT_03;                                      //配置为单脉冲模式                                         
	
	return DEF_OK;
}


/*
*********************************************************************************************************
*                                        Timing_Lock()
*
* Description : 进程独占该定时器，在一个完整的时序开头调用，防止其他进程占用定时器导致延时不精确
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
void Timing_Lock(void)
{	
	OS_ERR err;
	
	OSSemPend((OS_SEM *)&(TimingStatus.SemLock),                            //锁定该端口
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
}



/*
*********************************************************************************************************
*                                        Timing_Unlock()
*
* Description : 进程取消独占该定时器，在一个完整的时序结尾调用，防止其他进程死等
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void Timing_Unlock(void)
{
	OS_ERR err;
	
	OSSemPost((OS_SEM *)&(TimingStatus.SemLock),                           //释放该定时器
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);                                            
}


/*
*********************************************************************************************************
*                                        Timing_Delay_Us()
*
* Description : us级延时函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void Timing_Delay_Us(uint16_t count)     //2.166
{
	OS_ERR err;
	
	if (0 == count) {                                          //不需要延时
		return;
	}
	
	TIMING_TYPE -> PSC   = TIMING_PERIOD_US;                   //分频道0.25us
	
	if(count <= TIMING_NO_SCHED_MAX_US) {
		TIMING_TYPE->DIER  &= ~0x01;                                 //先关闭中断，防止中断提前释放信号量
		TIMING_TYPE -> ARR  = (count << TIMING_BASE_SHIFT)-1;
		TIMING_TYPE -> CR1 |= 0x01;                                  //使能计数器,放在此处防止计时太短立即进入中断
		while(0 == (TIMING_TYPE->SR&TIMING_UPDATE_MASK)) {           //读取标志位
		                                                             //等待计时结束
		}
		TIMING_TYPE->SR &= ~TIMING_UPDATE_MASK;                      //清除中断标志	
		TIMING_TYPE->DIER |= 0x01;                                   //开启中断，为大多数的长时间延时准备
		
	} else {                                                         //只有时间间隔较大时才阻塞
		TIMING_TYPE -> ARR   = ((count-6) << TIMING_BASE_SHIFT)-2;   //需要修正
		TIMING_TYPE -> CR1  |= 0x01;                                 //开启计数器
		OSSemPend((OS_SEM *)&(TimingStatus.SemWait),                 //阻塞到延时结束
				  (OS_TICK ) 0,
				  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
				  (CPU_TS *) 0,
				  (OS_ERR *) &err);
	}
}


/*
*********************************************************************************************************
*                                        Timing_Delay_Ms()
*
* Description : ms级延时函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void Timing_Delay_Ms(uint16_t count)
{	
	OS_ERR err;
	
	if (0 == count) {
		return;                                                //不延时直接返回
	}
	
	TIMING_TYPE -> PSC   = TIMING_PERIOD_MS;                   //时钟周期分频道0.25ms
	TIMING_TYPE -> ARR   = ((count << TIMING_BASE_SHIFT) - 1); //必须修正1才准确
	TIMING_TYPE -> CR1  |= 0x01;                               //使能计数器
	
	OSSemPend((OS_SEM *)&(TimingStatus.SemWait),               //阻塞到延时结束
		  (OS_TICK ) 0,
		  (OS_OPT  ) OS_OPT_PEND_BLOCKING,
		  (CPU_TS *) 0,
		  (OS_ERR *) &err);
}


/*
*********************************************************************************************************
*                                        Timing_Handler() 
*
* Description : 上溢中断处理函数
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : TIM2_IRQHandler()
*
* Note(s)     : none.
*********************************************************************************************************
*/


void  Timing_Handler (void)
{	
	OS_ERR   err;
		                           
	if(TIMING_TYPE->SR & TIMING_UPDATE_MASK) {                        //读取状态
		TIMING_TYPE->SR &= ~TIMING_UPDATE_MASK;                       //清除中断标志	
		TIMING_TYPE->CR1 &= ~0x01;                                    //失能计数器
		
		OSSemPost((OS_SEM *)&(TimingStatus.SemWait),                  //取消等待
	              (OS_OPT  ) OS_OPT_POST_1,
	              (OS_ERR *) &err);
	}
	                                                                  //在中断结束结束一定要保证进程调度
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
* Description : 将定时器连接到相应的中断
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : BSP_Timing_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  NVIC_Configuration (void)
{
	NVIC_InitTypeDef NvicInit;
	
	NvicInit.NVIC_IRQChannel = BSP_TIMING_IRQ;
	NvicInit.NVIC_IRQChannelSubPriority = BSP_TIMING_NVIC_SUB_PRIO;
	NvicInit.NVIC_IRQChannelPreemptionPriority = BSP_TIMING_NVIC_PRE_PRIO;
	NvicInit.NVIC_IRQChannelCmd = ENABLE;
	
	NVIC_Init(&NvicInit);
}





