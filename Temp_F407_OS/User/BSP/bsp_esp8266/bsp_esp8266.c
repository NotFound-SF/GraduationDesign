
/*
*********************************************************************************************************
*                           该问价负责操作wifi模块，但是并没有写所有的接口，只实现了TCP相关的
*                          所有函数都不是线程安全，所以多线程使用时需要上锁
*   
*********************************************************************************************************
*/


#include "bsp_esp8266.h"
#include  <stdlib.h>


/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static     char                   wifi_buf[RES_BUFF_LEN];                     //存储模块返回的信息

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static      void     BSP_ESP8266_Rst(void);
static      void     WriteCmd(char *cmd);
static      void     WriteData(uint8_t *dat, uint16_t len);
static      uint16_t ReadData(char *data, OS_TICK timeOut);




/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        BSP_ESP8266_Init()
*
* Description : 初始与该模块链接的串口，设置相应的波特率
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ESP8266_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        //普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;       //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;   //100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;         //上拉
	GPIO_Init(GPIOC,&GPIO_InitStructure);                // 
    GPIO_SetBits(GPIOC, GPIO_Pin_1); 
	
	//初始对应的串口
	
	//BSP_UART_Init(WIFI_UART_PORT, WIFI_UART_BAUD);
	
	//ReadData(wifi_buf, 1000);                             //等待开机结束
}



/*
*********************************************************************************************************
*                                        BSP_ESP8266_Server_Init()
*
* Description : 将wifi模块配置为TCP服务器，模式，但是最多只能连接5个设备，无数据交互8s后主动断开
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
int len_r;
void BSP_ESP8266_Server_Init(void) 
{
//	WriteCmd("ATE0");                                                  // 不回显 
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "ATE:%s\r\n", wifi_buf);
//	WriteCmd("AT+CWMODE=2");                                           // 设置为热点模式 
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "CWMODE:%s\r\n", wifi_buf);
//	//len_r = ReadData(wifi_buf, 1000);    	// 等待模块回应
//	BSP_ESP8266_Rst();                                                 // 需要重启
//	WriteCmd("ATE0");                                                  // 不回显
//    len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应	
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "ATE0: %s\r\n", wifi_buf);
//	//ReadData(wifi_buf, 1000);                                          // 等待模块回应
//	
//	WriteCmd("AT+CWSAP_DEF=\"ESP8266\",\"1234567890\",5,3");
//	//WriteCmd(AP_CONFIG);                                               // 配置热点信息    
//	//ReadData(wifi_buf, 1000);
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
//	WriteCmd("AT+CIPMUX=1");                                           // 服务器必须为多连接模式
//	//ReadData(wifi_buf, 1000);                                          // 等待模块回应
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
//	WriteCmd(AP_SERVER_CONFIG);                                        // 开启服务器服务器
//	//ReadData(wifi_buf, 1000);                                          // 等待模块回应
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
//	WriteCmd("AT+CIPSTO=80");                                           // 无数据交互8s后断开，最多连接5个设备
//	//ReadData(wifi_buf, 1000);  
//	len_r = ReadData(wifi_buf, 1000);                                  // 等待模块回应
//	wifi_buf[len_r] = 0;
//	BSP_UART_Printf(BSP_UART_ID_1, "%s\r\n", wifi_buf);
}



uint8_t BSP_ESP8266_Server_Read(uint8_t *data, uint8_t *id)
{
	char    lenStr[4];                                             // 所以读取个数不能超999byte
	uint8_t index = 0;
	uint8_t dataLen = 0, cpyIndex = 0, readLen;
	
	readLen = ReadData(wifi_buf, 0);                               // 永久等待
	if (0 == readLen)
		return 0;
	
	// 表示客户端连接或者断开
	if('0' <= wifi_buf[0] && wifi_buf[0] <= '4')
		return 0;
	
	for (index = 0; index < readLen; index++) {
		if (wifi_buf[index] == ',') {
			index++;
			*id = wifi_buf[index]-'0';
			break;
		}
	}
	
	// 获取数据的长度
	for (dataLen = 0; index < readLen; index++, dataLen++) {
		if (wifi_buf[index] == ',') {
			index++;
			lenStr[dataLen] = 0;                       // 做成字符串
			break;
		} else {
			lenStr[dataLen] = wifi_buf[index];         // 为后续调用atoi做准备
		}
	}
	
	// 提取真实数据
	for (dataLen = atoi(lenStr), cpyIndex = 0; index < dataLen; index++, cpyIndex++) {
		data[cpyIndex] = wifi_buf[index];
	}
	
	return dataLen;
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
*                                        WriteCmd()
*
* Description : 发送AT指令，自动追加回车换行，即传入参数不需要回车换行结束
*
* Return(s)   : none
*
* Argument(s) : cmd  要执行的命令
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  WriteCmd(char *cmd)
{
	BSP_UART_WrStr(WIFI_UART_PORT, cmd);
	BSP_UART_WrStr(WIFI_UART_PORT, "\r\n");
}


/*
*********************************************************************************************************
*                                        WriteData()
*
* Description : 直接发送数据，不对数据做任何处理
*
* Return(s)   : none
*
* Argument(s) : dat  要发送的数据
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  WriteData(uint8_t *dat, uint16_t len)
{
	BSP_UART_WrData(WIFI_UART_PORT, dat, len);
}


/*
*********************************************************************************************************
*                                        ReadData()
*
* Description : 接收模块返回的信息，阻塞等待
*
* Return(s)   : 实际读取到的字节长度
*
* Argument(s) : data     模块返回的信息
*             
*             ：timeOut  设置最长的读取等待时间
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  uint16_t  ReadData(char *data, OS_TICK timeOut) 
{
	return BSP_UART_RdData(WIFI_UART_PORT, (uint8_t*)data, WIFI_BUFF_LEN, timeOut);
}


/*
*********************************************************************************************************
*                                        BSP_ESP8266_Rst()
*
* Description : 复位WIFI模块
*
* Return(s)   : none
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void  BSP_ESP8266_Rst(void)
{	
	//初始对应的串口
	WriteCmd("AT+RST");
	
	ReadData(wifi_buf, 3000);                                                //等待复位结束
}




