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

static  OS_TCB   AppTaskStartTCB;                                           //��ʼ����������ƿ�

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

    OSInit(&err);                                               /* ��ʼ���ڲ��������ٴ�������������ʱ�ӽ�������            */

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
    OSStatTaskCPUUsageInit(&err);                               /* ����������ʱ����CPU����                                */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif
	
//	OSSchedRoundRobinCfg(DEF_TRUE, 10, &err);                  //����ʱ��Ƭ��ת����

	
	//�����ӽ��� SENSOR�ӽ���

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
	
	
	
	
    //�����ӽ��� UART1�ӽ���

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
				 
				 
   //�����ӽ��� LED1�ӽ���
	
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
	
	
	//���������������
	
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
	
	
	//����Ӧ������,emwin�Ĺٷ�ʾ������ GUIDEMO_Main
	
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
* Description : ���������ȡ�������������ݣ����뱣֤���������ȼ���ߣ�ֻ���������ܱ�֤�����ܶ�����������ʱ��
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
	
	FATFS fs;											    /* FatFs�ļ�ϵͳ���� */
	FIL fnew;											    /* �ļ����� */
	FRESULT res_flash;                                      /* �ļ�������� */
	UINT fnum;            					                /* �ļ��ɹ���д���� */
	BYTE *ReadBuffer = (BYTE*)BSP_SRAM_BASE;                /* �������� */
	BYTE WriteBuffer[] = "�½��ļ�ϵͳ�����ļ�\r\n";  
	
	//���ⲿSPI Flash�����ļ�ϵͳ���ļ�ϵͳ����ʱ���SPI�豸��ʼ��
	res_flash = f_mount(&fs,"1:",1);

	if(res_flash == FR_NO_FILESYSTEM)
	{
		BSP_UART_Printf(BSP_UART_ID_1,"��FLASH��û���ļ�ϵͳ���������и�ʽ��...\r\n");
    
		res_flash=f_mkfs("1:",0,0);							
		
		if(res_flash == FR_OK)
		{
			BSP_UART_Printf(BSP_UART_ID_1,"��FLASH�ѳɹ���ʽ���ļ�ϵͳ��\r\n");
			/* ��ʽ������ȡ������ */
			res_flash = f_mount(NULL,"1:",1);			
			/* ���¹���	*/			
			res_flash = f_mount(&fs,"1:",1);
		}
		else
		{
			BSP_UART_Printf(BSP_UART_ID_1,"������ʽ��ʧ�ܡ�����\r\n");
			while(1);
		}
	}
  else if(res_flash!=FR_OK)
  {
    BSP_UART_Printf(BSP_UART_ID_1,"�����ⲿFlash�����ļ�ϵͳʧ�ܡ�(%d)\r\n",res_flash);
    BSP_UART_Printf(BSP_UART_ID_1,"��������ԭ��SPI Flash��ʼ�����ɹ���\r\n");
	while(1);
  }
  else
  {
    BSP_UART_Printf(BSP_UART_ID_1,"���ļ�ϵͳ���سɹ������Խ��ж�д����\r\n");
  }
  
/*----------------------- �ļ�ϵͳ���ԣ�д���� -----------------------------*/
	/* ���ļ�������ļ��������򴴽��� */
	BSP_UART_Printf(BSP_UART_ID_1,"\r\n****** ���������ļ�д�����... ******\r\n");	
	res_flash = f_open(&fnew, "1:FatFs��д�����ļ�.txt",FA_CREATE_ALWAYS | FA_WRITE );
	if ( res_flash == FR_OK )
	{
		BSP_UART_Printf(BSP_UART_ID_1,"����/����FatFs��д�����ļ�.txt�ļ��ɹ������ļ�д�����ݡ�\r\n");
    /* ��ָ���洢������д�뵽�ļ��� */
		res_flash=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
    if(res_flash==FR_OK)
    {
     BSP_UART_Printf(BSP_UART_ID_1,"���ļ�д��ɹ���д���ֽ����ݣ�%d\n",fnum);
     BSP_UART_Printf(BSP_UART_ID_1,"�����ļ�д�������Ϊ��\r\n%s\r\n",WriteBuffer);
    }
    else
    {
      BSP_UART_Printf(BSP_UART_ID_1,"�����ļ�д��ʧ�ܣ�(%d)\n",res_flash);
    }    
		/* ���ٶ�д���ر��ļ� */
    f_close(&fnew);
	}
	else
	{	
		BSP_UART_Printf(BSP_UART_ID_1,"������/�����ļ�ʧ�ܡ�\r\n");
	}
	
/*------------------- �ļ�ϵͳ���ԣ������� ------------------------------------*/
	BSP_UART_Printf(BSP_UART_ID_1,"****** ���������ļ���ȡ����... ******\r\n");
	res_flash = f_open(&fnew, "1:FatFs��д�����ļ�.txt", FA_OPEN_EXISTING | FA_READ); 	 
	if(res_flash == FR_OK)
	{
		BSP_UART_Printf(BSP_UART_ID_1,"�����ļ��ɹ���\r\n");
		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
    if(res_flash==FR_OK)
    {
     BSP_UART_Printf(BSP_UART_ID_1,"���ļ���ȡ�ɹ�,�����ֽ����ݣ�%d\r\n",fnum);
      BSP_UART_Printf(BSP_UART_ID_1,"����ȡ�õ��ļ�����Ϊ��\r\n%s \r\n", ReadBuffer);	
    }
    else
    {
      BSP_UART_Printf(BSP_UART_ID_1,"�����ļ���ȡʧ�ܣ�(%d)\n",res_flash);
    }		
	}
	else
	{
		BSP_UART_Printf(BSP_UART_ID_1,"�������ļ�ʧ�ܡ�\r\n");
	}
	/* ���ٶ�д���ر��ļ� */
	f_close(&fnew);	
  
	/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
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
* Description : ��������������
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
		
//		 GUI_TOUCH_Exec();                     // ��ϸ����������
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
		
	/* ��ʾ���� */

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
* Description : �����������
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
		
	/* ��ʾ���� */

	while(DEF_ON) {
//		BSP_Turn_Motor(360, MOTOR_DIR_RIGHT);
		
		OSTimeDlyHMSM( 0, 0, 6, 0,
		               OS_OPT_TIME_HMSM_STRICT,
                       &err );
	}
	
}

