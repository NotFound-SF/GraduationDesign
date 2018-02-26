/*
*********************************************************************************************************
*                           该程序用于管理存储eeprom中的电话号码与wifi信息
*   对呀电话号码最多只能存储32个，电话起始地址的4个字节bit为1表示有电话号码，否则无电话号码
*   所有文件都不是线程安全，所以多线程使用时需要上锁
*   
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "eeprom_manager.h"


/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        phone_clean()
*
* Description : 清空eeprom中所有电话记录
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/


void phone_clean(void)
{
	uint32_t phoneHead = 0x00;   
	
	BSP_EEPROM_Wr((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
}	


/*
*********************************************************************************************************
*                                        phone_del()
*
* Description : 删除指定索引的电话号码
*
* Return(s)   : 成功返回1，失败返回0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_del(uint8_t index)
{
	uint32_t mask      = 0x01;
	uint32_t phoneHead = 0x00;
	
	if (index > 31)
		return 0;
	
	BSP_EEPROM_Rd((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
	
	// 表示该电话号码存在
	if (phoneHead & (mask << index)) {
		phoneHead &= ~(mask<<index);         
		BSP_EEPROM_Wr((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
		
		return 1;
	}
	
	return 1;
}


/*
*********************************************************************************************************
*                                        phone_add()
*
* Description : 向eeprom增加一条电话号码电话号码长度必须指定
*
* Return(s)   : 成功返回1，失败返回0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_add(uint8_t *body, uint8_t phoneLen)
{
	uint8_t  index;
	uint16_t destAddr;                               // 存入的目标地址
	uint32_t mask = 0x01;
	uint32_t phoneHead = 0x00;
	
	BSP_EEPROM_Rd((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
	if (0xFFFFFFFF == phoneHead || phoneLen > 11)    // 表明已经存满
		return 0;                      
	
	for (index = 0; index < 32; index++) {
		if (0 == (phoneHead & (mask<<index))) {
			destAddr = index*12+PHONE_ADDR_BODY;     // 计算目标存储地址
			phoneHead |= (mask<<index);              // 在头中注册
			
			BSP_EEPROM_Wr(&phoneLen, 1, destAddr);
			BSP_EEPROM_Wr(body, phoneLen, destAddr+1);			
			BSP_EEPROM_Wr((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
			
			return 1;
		}
	}
	
	return 0;
}



/*
*********************************************************************************************************
*                                        phone_get()
*
* Description : 获取指定位置的电话号码
*
* Return(s)   : 成功返回电话号码长度，失败返回0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_get(uint8_t *body, uint8_t index)
{
	uint8_t  phoneLen = 0;
	uint16_t destAddr;                               // 存入的目标地址
	uint32_t mask = 0x01;
	uint32_t phoneHead = 0x00;
	
	BSP_EEPROM_Rd((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
	if (0x00 == phoneHead || index > 31)             // 表明列表为空
		return 0;                      
	
	if (phoneHead & (mask<<index)) {                 // 表明存在该电话号码
		destAddr = index*12+PHONE_ADDR_BODY;         // 计算目标存储地址
		BSP_EEPROM_Rd(&phoneLen, 1, destAddr);       // 读取该条目长度
		BSP_EEPROM_Rd(body, phoneLen, destAddr+1);
		
		return phoneLen;
	}
	
	return 0;
}



/*
*********************************************************************************************************
*                                        wifi_clean(void);
*
* Description : 忘记wifi信息
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void    wifi_clean(void)
{
	uint8_t len = 0x00;
	
	BSP_EEPROM_Wr(&len, 1, WIFI_ADDR_SSID);
	BSP_EEPROM_Wr(&len, 1, WIFI_ADDR_PWD);
}	


/*
*********************************************************************************************************
*                                        wifi_getSSID();
*
* Description : 获取wifiSSID
*
* Return(s)   : 如果存在则返回wifi名长度否则返回0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t wifi_getSSID(uint8_t *ssid)
{
	uint8_t len = 0x00;
	
	BSP_EEPROM_Rd(&len, 1, WIFI_ADDR_SSID);
	
	if (0 == len)
		return 0;
	
	BSP_EEPROM_Rd(ssid, len, WIFI_ADDR_SSID+1);
	
	return len;
}



/*
*********************************************************************************************************
*                                        wifi_getPWD();
*
* Description : 获取wifiPWD
*
* Return(s)   : 成功者返回wifi密码长度，否则返回0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t wifi_getPWD(uint8_t *pwd)
{
	uint8_t len = 0x00;
	
	BSP_EEPROM_Rd(&len, 1, WIFI_ADDR_PWD);
	
	if (0 == len)
		return 0;
	
	BSP_EEPROM_Rd(pwd, len, WIFI_ADDR_PWD+1);
	
	return len;
}


/*
*********************************************************************************************************
*                                        wifi_setPWD();
*
* Description : 设置wifiPWD
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void    wifi_setPWD(uint8_t *pwd, uint8_t len)
{
	if (len > 30)
		return;
	
	BSP_EEPROM_Wr(&len, 1, WIFI_ADDR_PWD);
	BSP_EEPROM_Wr(pwd, len, WIFI_ADDR_PWD+1);
}	




/*
*********************************************************************************************************
*                                        wifi_setSSID();
*
* Description : 设置wifiSSID
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void    wifi_setSSID(uint8_t *ssid, uint8_t len)
{
	if (len > 30)
		return;
	
	BSP_EEPROM_Wr(&len, 1, WIFI_ADDR_SSID);
	BSP_EEPROM_Wr(ssid, len, WIFI_ADDR_SSID+1);
}	







