
/*
*********************************************************************************************************
*                    该文件只负责将ADC数据转换为电流值，ADC数据bsp_adc.x提供 
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_acs712.h"

  

/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             BSP_ACS_GetU_Real()
*
* Description : 获取传感器测得流过的电流值的实时大小
*
* Argument(s) : none.
*
* Return(s)   : 电流值的绝对值不带符号,单位A
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

float BSP_ACS_GetU_Real(void)
{
	float res, temp;
	
	// 当前在传感器端口检测到的电压值
	
	temp = (float)ADC_REFERENCE_VOL*BSP_ADC_GetDat(BSP_ADC_CH1)/ADC_RESOLUTION;
	
	// 转换为电流值
	
	res = (temp - CURRENT_ZERO_POINT)/VOL_TO_CURRENT;
	
	// 只返回正值,该传感器对电流方向敏感
	
	if (res < 0)
		res = -res;
	
	return res;
}



/*
*********************************************************************************************************
*                                             BSP_ACS_GetS_SReal()
*
* Description : 获取传感器测得流过的电流值,实时情况
*
* Argument(s) : none.
*
* Return(s)   : 电流值的大小带符号,单位A
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

float BSP_ACS_GetS_Real(void)
{
	float res, temp;
	
	// 当前在传感器端口检测到的电压值
	
	temp = (float)ADC_REFERENCE_VOL*BSP_ADC_GetDat(BSP_ADC_CH1)/ADC_RESOLUTION;
	
	// 转换为电流值
	
	res = (temp - CURRENT_ZERO_POINT)/VOL_TO_CURRENT;
	
	return res;
}


/*
*********************************************************************************************************
*                                             BSP_ACS_Get_AC()
*
* Description : 多次测量取得最电流的最大值然后除以1.4142即可获得正弦交流电电流的有效值
*
* Argument(s) : none.
*
* Return(s)   : 电流值的大小,单位A
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

float BSP_ACS_Get_AC(void)
{
	CPU_ERR  err;
	float    res;
	uint16_t max = 0, temp;
	uint16_t index;
	
	for (index = 0; index < AC_SAMPLE_COUNT; index++) {
		
		temp = BSP_ADC_GetDat(BSP_ADC_CH1);
		
		if (temp < CURRENT_ZERO_ADC) {                             // 电流方向为负
			temp = CURRENT_ZERO_ADC - temp;
		} else {                                                   // 电流方向为正
			temp = temp - CURRENT_ZERO_ADC;
		}
		
		if (max < temp) {
			max = temp;
		}
		
		// 采样时间间隔1ms
		
		OSTimeDly((OS_TICK )  1, 
				  (OS_OPT  )  OS_OPT_TIME_DLY, 
				  (OS_ERR *)& err);
	}
	
	res = ((float)ADC_REFERENCE_VOL * max / ADC_RESOLUTION - CURRENT_ZERO_POINT)/VOL_TO_CURRENT;
	
	return res;
	
}



