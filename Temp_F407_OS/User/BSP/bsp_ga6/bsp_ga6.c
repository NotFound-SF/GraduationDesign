/*
*********************************************************************************************************
*			该文件将使用uart端口控制GSM模块，根据需要主要完成发短信先关操作
*           移植是只需要修改底层函数 WriteCmd();ReadData();
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_ga6.h"
#include "string.h"


/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static     OS_SEM                 SemLock;                     //确保同时只能有一个进程能够操作该模块
static     char                   Res[RES_BUFF_LEN];                     //存储模块返回的信息

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
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
*                                        BSP_GA6_Init()
*
* Description : 初始与该模块链接的串口，设置相应的波特率
*
* Return(s)   : DEF_OK    正常初始化
*               DEF_FAIL  初始化失败
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_Init(void)
{
	OS_ERR      err;
	
	// 创建独占信号量
	
	OSSemCreate((OS_SEM    *)&SemLock,                                   //创建互斥信号量
                (CPU_CHAR  *)"GA6 SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
	
	//初始对应的串口
	
	BSP_UART_Init(GA6_UART_PORT,GA6_UART_BAUD);
	
	ReadData(Res, 1000);                                                //等待开机结束
	
	return DEF_OK;
}



/*
*********************************************************************************************************
*                                        BSP_GA6_GetRegSt()
*
* Description : 获取该模块的注册信息,务必先关闭回显
*
* Return(s)   : REG_NO_CARD    未正确插入SIM卡
*               REG_NO_SING    无信号
*               REG_NORMAL     SIM卡注册成功
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/
GA6_Reg_Status BSP_GA6_GetRegSt(void) 
{
	OS_ERR           err;
	GA6_Reg_Status   status = REG_NORMAL;
	
	OSSemPend((OS_SEM *)&SemLock,                  //因为有全局变量，并且要一气呵成，所以要信号量
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);	
	
	// 取消回显，后面的命令解析都需要先取消回显才能正常工作
	
	WriteCmd("ATE0");   
	ReadData(Res, 1000);
	
	// 查询状态
	
	WriteCmd("AT+CREG?");                          //注册状态查询指令
	ReadData(Res, 1000);                           //读取模块返回的数据
		
	// 解析返回的数据
	switch (Res[11]) {
		case '1':
			status = REG_NORMAL;
			break;
		
		case '2':
			status = REG_NO_SING;
			break;
		
		case '3':
			status = REG_NO_CARD;
			break;
		
		case '4':
			status = REG_UNKNOWN;
			break;
		
		case '5':
			status = REG_ROAM;
			break;
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);  
	
	return status;
}



/*
*********************************************************************************************************
*                                        BSP_GA6_GetSigQu()
*
* Description : 获取该模块的信号强度，当无信号时返回0，有信号时返回信号强度
*
* Return(s)   : 信号强度
*
* Caller(s)   : Application
*
* Note(s)     : 必须是非回显模式
*********************************************************************************************************
*/

uint8_t BSP_GA6_GetSigQu(void)
{
	uint8_t signal = 0;
	OS_ERR  err;
	
	OSSemPend((OS_SEM *)&SemLock,                      //必须放在里面否则造成死锁
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);	
	
	// 取消回显，后面的命令解析都需要先取消回显才能正常工作
	
	WriteCmd("ATE0");   
	ReadData(Res, 1000);
	
	// 先获取注册信息
	
	WriteCmd("AT+CREG?");                              //注册状态查询指令
	ReadData(Res, 1000);                               //读取模块返回的数据
	
	if ('1' == Res[11]) {                              //必须要注册成功再检测信号强度,否则信号强度无意义
		WriteCmd("AT+CSQ");                            //获取信号强度
		ReadData(Res, 1000);                           //读取模块返回的数据
		
		// 解析信号强度
		
		if (',' != Res[9]) {                           //表示信号强度为两位数
			signal = (Res[8]-'0')*10 + (Res[9]-'0');
		} else {
			signal = Res[8]-'0';                       //信号很弱只有一位数，一般不可能
		}
		
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);  
	
	return signal;
}


/*
*********************************************************************************************************
*                                        BSP_GA6_SenTextMSG()
*
* Description : 该函数用于发送英文短信
*
* Argument(s) : num       目标电话号码
*             : str       短信内容
*
* Return(s)   : DEF_OK    发送短信成功
*               DEF_FAIL  发送短信失败
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN BSP_GA6_SenTextMSG(char *num, char *str) 
{
	uint8_t      n;
	char         *ptr = "AT+CMGS=\"";                  //用于合成电话号码
	uint8_t      end = 0x1A;                           //短息发送结束标志
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //锁定资源，保证同时只有一个进程发送短信
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	// 取消回显，后面的命令解析都需要先取消回显才能正常工作
	
	WriteCmd("ATE0");   
	ReadData(Res, 1000);
	
	// 设置短信格式为TEXT格式
	
	WriteCmd("AT+CMGF=1");   
	ReadData(Res, 1000);
	
	// 设置短信字符格式为GSM格式
	
	WriteCmd("AT+CSCS=\"GSM\"");   
	ReadData(Res, 1000);
	
	// 设置目标电话号码
	WriteData((uint8_t *)ptr, strlen(ptr));                   //发送AT+CMGS="
	WriteData((uint8_t *)num, strlen(num));                  
	WriteCmd("\"");                       
	ReadData(Res, 1000);
	
	// 发送实际内容
	
	WriteData((uint8_t *)str, strlen(str));
	WriteData(&end, 1);                                       //必须以此结束
	n = ReadData(Res, 10000);                                 //发送短信需要等待较长时间
	
	if (0 != n) {
		
		// 解析恢复的数据
		Res[12] = '\0';                                       //返回的数据并不是标准的字符串 
		
		if(0 == strcmp(Res+7, "ERROR")) {                     //出错反复\r\n+CMS ERROR:500
			status = DEF_FAIL;
		} else {
			status = DEF_OK;
		}
	}
		
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err);  
	
	
	return status;
}






/*
*********************************************************************************************************
*                                        BSP_GA6_CheckCall()
*
* Description : 该函数检测是否有电话呼入，因为每次提醒时间间隔比较长，所以该函数时间最长执行也需要1s左右
*
* Argument(s) : none
*
* Return(s)   : DEF_OK    表示有电话呼入
*               DEF_FAIL  表示无电话
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_CheckCall(void)
{
	uint8_t      n;
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //锁定资源，保证同时只有一个进程发送短信
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	n = ReadData(Res, 1000);                           //等待读取1s数据
	if (0 != n) {
		
		Res[6] = '\0';
		
		if (0 == strcmp(Res+2, "RING")) {
			status = DEF_OK;
		} else {
			status = DEF_FAIL;
		}
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err); 
	
	return status;
}


/*
*********************************************************************************************************
*                                        BSP_GA6_CheckEx()
*
* Description : 模块存在性检测，检测UART与模块链接是否成功
*
* Argument(s) : none
*
* Return(s)   : DEF_OK    表示链接成功
*               DEF_FAIL  表示链接失败
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_GA6_CheckEx(void)
{
	uint8_t      n;
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //锁定资源，保证同时只有一个进程发送短信
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	WriteCmd("AT");
	n = ReadData(Res, 200);                           
	if (0 != n) {
		status = DEF_OK;
	}
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err); 
	
	return status;
}


/*
*********************************************************************************************************
*                                        BSP_GA6_CallHang()
*
* Description : 挂断电话
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

CPU_BOOLEAN  BSP_GA6_CallHang(void)
{
	OS_ERR       err;
	CPU_BOOLEAN  status = DEF_FAIL;
	
	OSSemPend((OS_SEM *)&SemLock,                      //锁定资源，保证同时只有一个进程发送短信
	          (OS_TICK ) 0,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	WriteCmd("ATH");
	ReadData(Res, 600);                           
	
	OSSemPost((OS_SEM *)&SemLock,                           
	          (OS_OPT  ) OS_OPT_POST_1,
	          (OS_ERR *) &err); 
	
	return status;
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
	BSP_UART_WrStr(GA6_UART_PORT, cmd);
	BSP_UART_WrStr(GA6_UART_PORT, "\r\n");
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
	BSP_UART_WrData(GA6_UART_PORT, dat, len);
}




/*
*********************************************************************************************************
*                                        ReadData()
*
* Description : 接收模块返回的信息，阻塞等待
*
* Return(s)   : none
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
	return BSP_UART_RdData(GA6_UART_PORT, (uint8_t*)data, RES_BUFF_LEN, timeOut);
}
