/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : YS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>

#include "ff.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   AppTaskStartTCB;                                           //开始任务任务控制块

static  OS_TCB   AppTaskLed1TCB;

static  OS_TCB   AppTaskTouchTCB;

static  OS_TCB   AppTaskSensorTCB;   

static  OS_TCB   AppTaskGUIDemoTCB;

static  OS_TCB   AppTaskMotorTCB;






/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  AppTaskSensorStk[APP_TASK_SENSOR_STK_SIZE];

static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];

static  CPU_STK  AppTaskLed1Stk[APP_TASK_LED1_STK_SIZE];

static  CPU_STK  AppTaskTouchStk[APP_TASK_TOUCH_STK_SIZE];

static  CPU_STK  AppTaskGUIDemoStk[APP_TASK_GUI_DEMO_STK_SIZE];

static  CPU_STK  AppTaskMotorStk[APP_TASK_MOTOR_STK_SIZE];






/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskSensor (void *p_arg);

static  void  AppTaskStart  (void *p_arg);

static  void  AppTaskLed1   (void *p_arg);

static  void  AppTaskTouch  (void *p_arg);

static  void  AppTaskMotor  (void *p_arg);



/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR  err;
	
    CPU_Init();                                                 /* Initialize the uC/CPU Services                       */
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */

    OSInit(&err);                                               /* 初始化内部变量至少创建空闲任务与时钟节拍任务            */

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR ) AppTaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
				 
	(void)&err;

    return (0u);
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    OS_ERR      err;

   (void)p_arg;
	
  
	BSP_Tick_Init();                                            /* Initialize systick                                   */
	BSP_Init();                                                 /* Initialize BSP functions                             */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* 无任务运行时计算CPU容量                                */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
	
//	OSSchedRoundRobinCfg(DEF_TRUE, 10, &err);                  //配置时间片轮转调度

	
	//创建子进程 SENSOR子进程

	OSTaskCreate((OS_TCB     *)&AppTaskSensorTCB,              /* Create the Sensor task                                 */
				(CPU_CHAR   *)"App Task SENSOR",
				(OS_TASK_PTR ) AppTaskSensor,
				(void       *) 0,
				(OS_PRIO     ) APP_TASK_SENSOR_PRIO,
				(CPU_STK    *)&AppTaskSensorStk[0],
				(CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE / 10,
				(CPU_STK_SIZE) APP_TASK_SENSOR_STK_SIZE,
				(OS_MSG_QTY  ) 5u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err==OS_ERR_NONE) {
		BSP_UART_Printf(BSP_UART_ID_1, "OK");
	}
	
	
	
	
    //创建子进程 UART1子进程

	OSTaskCreate((OS_TCB     *)&AppTaskTouchTCB,              /* Create the LED1 task                                 */
				(CPU_CHAR   *)"App Task Touch",
				(OS_TASK_PTR ) AppTaskTouch,
				(void       *) 0,
				(OS_PRIO     ) APP_TASK_TOUCH_PRIO,
				(CPU_STK    *)&AppTaskTouchStk[0],
				(CPU_STK_SIZE) APP_TASK_TOUCH_STK_SIZE / 10,
				(CPU_STK_SIZE) APP_TASK_TOUCH_STK_SIZE,
				(OS_MSG_QTY  ) 5u,
				(OS_TICK     ) 0u,
				(void       *) 0,
				(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);
			 
	if(err==OS_ERR_NONE) {
		BSP_UART_Printf(BSP_UART_ID_1, "OK");
	}
				 
				 
   //创建子进程 LED1子进程
	
    OSTaskCreate((OS_TCB     *)&AppTaskLed1TCB,              /* Create the LED1 task                                 */
	  		    (CPU_CHAR   *)"App Task Led1",
			    (OS_TASK_PTR ) AppTaskLed1,
			    (void       *) 0,
			    (OS_PRIO     ) APP_TASK_LED1_PRIO,
			    (CPU_STK    *)&AppTaskLed1Stk[0],
			    (CPU_STK_SIZE) APP_TASK_LED1_STK_SIZE / 10,
			    (CPU_STK_SIZE) APP_TASK_LED1_STK_SIZE,
			    (OS_MSG_QTY  ) 5u,
			    (OS_TICK     ) 0u,
			    (void       *) 0,
			    (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
			    (OS_ERR     *)&err);
			 
    if(err==OS_ERR_NONE) {
		BSP_UART_Printf(BSP_UART_ID_1, "OK");
    }
	
	
	//创建步进电机任务
	
	OSTaskCreate((OS_TCB     *)&AppTaskMotorTCB,                                        
				(CPU_CHAR   *)"Motor", 									                     
				(OS_TASK_PTR ) AppTaskMotor,									                        
			    (void       *) 0,																
				(OS_PRIO     ) APP_TASK_MOTOR_PRIO,					
				(CPU_STK    *)&AppTaskMotorStk[0],						
				(CPU_STK_SIZE) APP_TASK_MOTOR_STK_SIZE / 10,				
			    (CPU_STK_SIZE) APP_TASK_MOTOR_STK_SIZE,        		
				(OS_MSG_QTY  ) 0u,
				(OS_TICK     ) 0u,
				(void       *) 0,
	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
				(OS_ERR     *)&err);															

	if(err==OS_ERR_NONE) {
		BSP_UART_Printf(BSP_UART_ID_1, "OK");
    }
	
	
	//创建应用任务,emwin的官方示例函数 GUIDEMO_Main
	
//	OSTaskCreate((OS_TCB     *)&AppTaskGUIDemoTCB,                                        
//				(CPU_CHAR   *)"GUI Demo Test", 									                     
//				(OS_TASK_PTR ) MainTask,									                        
//			    (void       *) 0,																
//				(OS_PRIO     ) APP_TASK_GUI_DEMO_PRIO,					
//				(CPU_STK    *)&AppTaskGUIDemoStk[0],						
//				(CPU_STK_SIZE) APP_TASK_GUI_DEMO_STK_SIZE / 10,				
//			    (CPU_STK_SIZE) APP_TASK_GUI_DEMO_STK_SIZE,        		
//				(OS_MSG_QTY  ) 0u,
//				(OS_TICK     ) 0u,
//				(void       *) 0,
//	     		(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
//				(OS_ERR     *)&err);															

//	if(err==OS_ERR_NONE) {
//		BSP_UART_Printf(BSP_UART_ID_1, "OK");
//    }
	
    OSTaskDel(&AppTaskStartTCB, &err);
}



/*
*********************************************************************************************************
*                                          SENSOR TASK
*
* Description : 该任务负责读取各个传感器数据，必须保证该任务优先级最高，只有这样才能保证任务能都产生正常的时序
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskSensor (void *p_arg)
{
	OS_ERR     err;
	
	FATFS fs;											    /* FatFs文件系统对象 */
	FIL fnew;											    /* 文件对象 */
	FRESULT res_flash;                                      /* 文件操作结果 */
	UINT fnum;            					                /* 文件成功读写数量 */
	BYTE *ReadBuffer = (BYTE*)BSP_SRAM_BASE;                /* 读缓冲区 */
	BYTE WriteBuffer[] = "新建文件系统测试文件\r\n";  
	
	//在外部SPI Flash挂载文件系统，文件系统挂载时会对SPI设备初始化
	res_flash = f_mount(&fs,"1:",1);

	if(res_flash == FR_NO_FILESYSTEM)
	{
		BSP_UART_Printf(BSP_UART_ID_1,"》FLASH还没有文件系统，即将进行格式化...\r\n");
    
		res_flash=f_mkfs("1:",0,0);							
		
		if(res_flash == FR_OK)
		{
			BSP_UART_Printf(BSP_UART_ID_1,"》FLASH已成功格式化文件系统。\r\n");
			/* 格式化后，先取消挂载 */
			res_flash = f_mount(NULL,"1:",1);			
			/* 重新挂载	*/			
			res_flash = f_mount(&fs,"1:",1);
		}
		else
		{
			BSP_UART_Printf(BSP_UART_ID_1,"《《格式化失败。》》\r\n");
			while(1);
		}
	}
  else if(res_flash!=FR_OK)
  {
    BSP_UART_Printf(BSP_UART_ID_1,"！！外部Flash挂载文件系统失败。(%d)\r\n",res_flash);
    BSP_UART_Printf(BSP_UART_ID_1,"！！可能原因：SPI Flash初始化不成功。\r\n");
	while(1);
  }
  else
  {
    BSP_UART_Printf(BSP_UART_ID_1,"》文件系统挂载成功，可以进行读写测试\r\n");
  }
  
/*----------------------- 文件系统测试：写测试 -----------------------------*/
	/* 打开文件，如果文件不存在则创建它 */
	BSP_UART_Printf(BSP_UART_ID_1,"\r\n****** 即将进行文件写入测试... ******\r\n");	
	res_flash = f_open(&fnew, "1:FatFs读写测试文件.txt",FA_CREATE_ALWAYS | FA_WRITE );
	if ( res_flash == FR_OK )
	{
		BSP_UART_Printf(BSP_UART_ID_1,"》打开/创建FatFs读写测试文件.txt文件成功，向文件写入数据。\r\n");
    /* 将指定存储区内容写入到文件内 */
		res_flash=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
    if(res_flash==FR_OK)
    {
     BSP_UART_Printf(BSP_UART_ID_1,"》文件写入成功，写入字节数据：%d\n",fnum);
     BSP_UART_Printf(BSP_UART_ID_1,"》向文件写入的数据为：\r\n%s\r\n",WriteBuffer);
    }
    else
    {
      BSP_UART_Printf(BSP_UART_ID_1,"！！文件写入失败：(%d)\n",res_flash);
    }    
		/* 不再读写，关闭文件 */
    f_close(&fnew);
	}
	else
	{	
		BSP_UART_Printf(BSP_UART_ID_1,"！！打开/创建文件失败。\r\n");
	}
	
/*------------------- 文件系统测试：读测试 ------------------------------------*/
	BSP_UART_Printf(BSP_UART_ID_1,"****** 即将进行文件读取测试... ******\r\n");
	res_flash = f_open(&fnew, "1:FatFs读写测试文件.txt", FA_OPEN_EXISTING | FA_READ); 	 
	if(res_flash == FR_OK)
	{
		BSP_UART_Printf(BSP_UART_ID_1,"》打开文件成功。\r\n");
		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
    if(res_flash==FR_OK)
    {
     BSP_UART_Printf(BSP_UART_ID_1,"》文件读取成功,读到字节数据：%d\r\n",fnum);
      BSP_UART_Printf(BSP_UART_ID_1,"》读取得的文件数据为：\r\n%s \r\n", ReadBuffer);	
    }
    else
    {
      BSP_UART_Printf(BSP_UART_ID_1,"！！文件读取失败：(%d)\n",res_flash);
    }		
	}
	else
	{
		BSP_UART_Printf(BSP_UART_ID_1,"！！打开文件失败。\r\n");
	}
	/* 不再读写，关闭文件 */
	f_close(&fnew);	
  
	/* 不再使用文件系统，取消挂载文件系统 */
	f_mount(NULL,"1:",1);
	


	(void)p_arg;	
	
	
//	BSP_UART_Printf(BSP_UART_ID_1, "\r\nFlashID is 0x%X,  Manufacturer Device ID is 0x%X\r\n", FlashID, DeviceID);

	
	


	while (DEF_ON) {
					
	
		OSTimeDlyHMSM( 0, 0, 2, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );

//		if(DEF_OK == BSP_18B20_GetTemp(&temp)) {
//			BSP_UART_Printf(BSP_UART_ID_1, "Temp: %.4f\n", BSP_18B20_TempTran(temp));
//		}
	}
}




/*
*********************************************************************************************************
*                                          Touch TASK
*
* Description : 触摸屏更新任务
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/
static  void  AppTaskTouch (void *p_arg)
{
	OS_ERR  err;
	
	(void)p_arg;

	while(DEF_ON) {
		OSTimeDlyHMSM( 0, 0, 0, 10,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
		
//		 GUI_TOUCH_Exec();                     // 更细触摸屏数据
	}
	
}





/*
*********************************************************************************************************
*                                          TASK
*
* Description : 
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskLed1   (void *p_arg)
{

	OS_ERR          err;
	
	(void) p_arg;
		
	/* 显示测试 */

	while(DEF_ON) {
		
		
//		BSP_UART_Printf(BSP_UART_ID_1,"CH1: %d\r\n", BSP_ADC_GetDat(BSP_ADC_CH1));
//		BSP_UART_Printf(BSP_UART_ID_1,"CH2: %d\r\n", BSP_ADC_GetDat(BSP_ADC_CH2));
//		BSP_UART_Printf(BSP_UART_ID_1,"CH3: %d\r\n\r\n", BSP_ADC_GetDat(BSP_ADC_CH3));
		
		
//		BSP_UART_Printf(BSP_UART_ID_1, "i: %f\r\n", BSP_ACS_GetS_Real());
//		BSP_LED_Toggle(1);
		OSTimeDlyHMSM( 0, 0, 0, 200,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
}


/*
*********************************************************************************************************
*                                          Motor TASK
*
* Description : 步进电机任务
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : none
*********************************************************************************************************
*/

static  void  AppTaskMotor  (void *p_arg)
{
	OS_ERR      err;
	
	(void) p_arg;
		
	/* 显示测试 */

	while(DEF_ON) {
//		BSP_Turn_Motor(360, MOTOR_DIR_RIGHT);
		
		OSTimeDlyHMSM( 0, 0, 6, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
}

