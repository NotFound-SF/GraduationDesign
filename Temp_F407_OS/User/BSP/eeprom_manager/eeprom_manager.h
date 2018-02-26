#ifndef   __EEPROM_MANAGER_H
#define   __EEPROM_MANAGER_H

/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_eeprom.h"


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define      PHONE_ADDR_BASE                62        // ��ʼ��ַ
#define      PHONE_ADDR_BODY                66        // ��ȥͷ�Ŀռ�
#define      WIFI_ADDR_SSID                 0         // wifi����ʼ��ַ
#define      WIFI_ADDR_PWD                  31        // wifi������ʼ��ַ


/*
*********************************************************************************************************
*                                               FUNCTIONS
*********************************************************************************************************
*/

void phone_clean(void);
uint8_t phone_del(uint8_t index);
uint8_t phone_add(uint8_t *body, uint8_t phoneLen);
uint8_t phone_get(uint8_t *body, uint8_t index);

void    wifi_clean(void);
uint8_t wifi_getSSID(uint8_t *ssid);
uint8_t wifi_getPWD(uint8_t *pwd);
void    wifi_setPWD(uint8_t *pwd, uint8_t len);
void    wifi_setSSID(uint8_t *ssid, uint8_t len);








#endif     /* __EEPROM_MANAGER_H */







