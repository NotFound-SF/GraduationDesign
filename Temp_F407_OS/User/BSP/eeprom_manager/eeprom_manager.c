/*
*********************************************************************************************************
*                           �ó������ڹ���洢eeprom�еĵ绰������wifi��Ϣ
*   ��ѽ�绰�������ֻ�ܴ洢32�����绰��ʼ��ַ��4���ֽ�bitΪ1��ʾ�е绰���룬�����޵绰����
*   �����ļ��������̰߳�ȫ�����Զ��߳�ʹ��ʱ��Ҫ����
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
* Description : ���eeprom�����е绰��¼
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
* Description : ɾ��ָ�������ĵ绰����
*
* Return(s)   : �ɹ�����1��ʧ�ܷ���0
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
	
	// ��ʾ�õ绰�������
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
* Description : ��eeprom����һ���绰����绰���볤�ȱ���ָ��
*
* Return(s)   : �ɹ�����1��ʧ�ܷ���0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_add(uint8_t *body, uint8_t phoneLen)
{
	uint8_t  index;
	uint16_t destAddr;                               // �����Ŀ���ַ
	uint32_t mask = 0x01;
	uint32_t phoneHead = 0x00;
	
	BSP_EEPROM_Rd((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
	if (0xFFFFFFFF == phoneHead || phoneLen > 11)    // �����Ѿ�����
		return 0;                      
	
	for (index = 0; index < 32; index++) {
		if (0 == (phoneHead & (mask<<index))) {
			destAddr = index*12+PHONE_ADDR_BODY;     // ����Ŀ��洢��ַ
			phoneHead |= (mask<<index);              // ��ͷ��ע��
			
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
* Description : ��ȡָ��λ�õĵ绰����
*
* Return(s)   : �ɹ����ص绰���볤�ȣ�ʧ�ܷ���0
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8_t phone_get(uint8_t *body, uint8_t index)
{
	uint8_t  phoneLen = 0;
	uint16_t destAddr;                               // �����Ŀ���ַ
	uint32_t mask = 0x01;
	uint32_t phoneHead = 0x00;
	
	BSP_EEPROM_Rd((uint8_t*)(&phoneHead), 4, PHONE_ADDR_BASE);
	if (0x00 == phoneHead || index > 31)             // �����б�Ϊ��
		return 0;                      
	
	if (phoneHead & (mask<<index)) {                 // �������ڸõ绰����
		destAddr = index*12+PHONE_ADDR_BODY;         // ����Ŀ��洢��ַ
		BSP_EEPROM_Rd(&phoneLen, 1, destAddr);       // ��ȡ����Ŀ����
		BSP_EEPROM_Rd(body, phoneLen, destAddr+1);
		
		return phoneLen;
	}
	
	return 0;
}



/*
*********************************************************************************************************
*                                        wifi_clean(void);
*
* Description : ����wifi��Ϣ
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
* Description : ��ȡwifiSSID
*
* Return(s)   : ��������򷵻�wifi�����ȷ��򷵻�0
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
* Description : ��ȡwifiPWD
*
* Return(s)   : �ɹ��߷���wifi���볤�ȣ����򷵻�0
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
* Description : ����wifiPWD
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
* Description : ����wifiSSID
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







