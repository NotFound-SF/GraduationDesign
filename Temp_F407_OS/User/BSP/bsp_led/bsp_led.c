


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include"bsp_led.h"

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_LED_Init()
*
* Description : 初始化LED端口引脚为推完输出模式
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : main().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Init (void)
{
	GPIO_InitTypeDef  gpio_init;
	
	RCC_AHB1PeriphClockCmd(LED12_GPIO_CLK, ENABLE);    
	RCC_AHB1PeriphClockCmd(LED3_GPIO_CLK, ENABLE);  
	
	gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	gpio_init.GPIO_Speed = GPIO_Speed_25MHz;

	gpio_init.GPIO_Pin   = LED1_PIN|LED2_PIN;
	GPIO_Init(LED12_GPIO_PORT, &gpio_init);

	gpio_init.GPIO_Pin   = LED3_PIN;
	GPIO_Init(LED3_GPIO_PORT, &gpio_init);
	
	BSP_LED_Off(0);                                       /* 开机后确保所有LED都关闭                          */                                    
}

/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : 开启LED
*
* Argument(s) : led    led表示为要开启led的编号[0-8]:
*
*                      0   开启所有LED
*                      1   开启LED1
*                      2   开启LED2
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
void  BSP_LED_On (uint8_t led)
{
	switch (led) {
		
		case 0:
			GPIO_ResetBits(LED12_GPIO_PORT, LED1_PIN|LED2_PIN);
			GPIO_ResetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		case 1:
			GPIO_ResetBits(LED12_GPIO_PORT, LED1_PIN);
			break;
		
		case 2:
			GPIO_ResetBits(LED12_GPIO_PORT, LED2_PIN);
			break;
		
		case 3:
			GPIO_ResetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		default:
			break;
	}
}


/*
*********************************************************************************************************
*                                             BSP_LED_Off()
*
* Description : 关闭LED
*
* Argument(s) : led    led表示为要关闭led的编号[0-8]:
*
*                      0   关闭所有LED
*                      1   关闭LED1
*                      2   关闭LED2
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Off(uint8_t led)
{
	switch (led) {
		case 0:
			GPIO_SetBits(LED12_GPIO_PORT, LED1_PIN|LED2_PIN);
			GPIO_SetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		case 1:
			GPIO_SetBits(LED12_GPIO_PORT, LED1_PIN);
			break;
		
		case 2:
			GPIO_SetBits(LED12_GPIO_PORT, LED2_PIN);
			break;
		
		case 3:
			GPIO_SetBits(LED3_GPIO_PORT, LED3_PIN);
			break;
		
		default:
			break;
	}
	
}


/*
*********************************************************************************************************
*                                             BSP_LED_Toggle()
*
* Description : 改变LED状态
*
* Argument(s) : led    led表示为要改变状态led的编号[0-8]:
*
*                      0   改变所有LED
*                      1   改变LED1
*                      2   改变LED2
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_LED_Toggle(uint8_t led)
{
	uint32_t pins_12, pins_3;
	
	pins_12 = GPIO_ReadOutputData(LED12_GPIO_PORT);                               /* 读出端口当前输出数据                      */
	pins_3  = GPIO_ReadOutputData(LED3_GPIO_PORT); 
	
	switch (led) {
		case 0:
			GPIO_ResetBits(LED12_GPIO_PORT, pins_12 & (LED1_PIN|LED2_PIN));       /* 只修改LED所在管脚，不修改无关管脚          */
			GPIO_SetBits(LED12_GPIO_PORT, (~pins_12) & (LED1_PIN|LED2_PIN));      /* 只修改LED所在管脚，不修改无关管脚          */
			
			GPIO_ResetBits(LED3_GPIO_PORT, pins_3 & LED3_PIN);                    /* 只修改LED所在管脚，不修改无关管脚          */
			GPIO_SetBits(LED3_GPIO_PORT, (~pins_3) & LED3_PIN);                   /* 只修改LED所在管脚，不修改无关管脚          */
			break;
		
		case 1:
			GPIO_ResetBits(LED12_GPIO_PORT, pins_12 & LED1_PIN);                  /* 只修改LED所在管脚，不修改无关管脚          */
			GPIO_SetBits(LED12_GPIO_PORT, (~pins_12) & LED1_PIN); 
			break;
		
		case 2:
			GPIO_ResetBits(LED12_GPIO_PORT, pins_12 & LED2_PIN);                  /* 只修改LED所在管脚，不修改无关管脚          */
			GPIO_SetBits(LED12_GPIO_PORT, (~pins_12) & LED2_PIN); 
			break;
		
		case 3:
			GPIO_ResetBits(LED3_GPIO_PORT, pins_3 & LED3_PIN);                    /* 只修改LED所在管脚，不修改无关管脚          */
			GPIO_SetBits(LED3_GPIO_PORT, (~pins_3) & LED3_PIN); 
			break;
		
		default:
			break;
	}
}
	

