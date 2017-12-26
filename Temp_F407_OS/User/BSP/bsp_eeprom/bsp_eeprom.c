
/*
*********************************************************************************************************
*                           该程序用于读写eeprom（兼容AT24C 01 02 04 08 16系列）
*   兼容不同的errprom只需要修改bsp_eeprom.h中宏定义型号例如 #define  AT_24C16即兼容24C16，值得注意的是
*   不同型号的设备地址使用的Ax使用不同，所以在定义设备地址EEPROM_DEV_ADDR时必须保证为使用的位为0，否则寻
*   址会重叠；
*   该文件只有初始化与连续读写函数，在移植时只需要修改 EEPROM_I2C_INIT，EEPROM_I2C_WrRd，EEPROM_I2C_Wr
*   函数其均与I2C相关
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/

#include "bsp_eeprom.h"


/*
*********************************************************************************************************
*                                       LOCAL VARIABLES
*********************************************************************************************************
*/

static     OS_SEM                 SemLock;                     //确保eeprom被互斥的读写

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static     CPU_BOOLEAN    EEPROM_I2C_INIT (void); 

static     CPU_BOOLEAN    EEPROM_I2C_WrRd (CPU_INT08U   i2c_addr,
										   CPU_INT08U  *p_buf,
                                           CPU_INT16U   nbr_bytes);

static     CPU_BOOLEAN    EEPROM_I2C_Wr (CPU_INT08U   i2c_addr,
										 CPU_INT08U  *p_buf,
										 CPU_INT16U   nbr_bytes);



/*
*********************************************************************************************************
*********************************************************************************************************
**                                         GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        BSP_EEPROM_Init()
*
* Description : Initialize the EEPROM_I2C_PORT.
*
* Return(s)   : DEF_OK    正常初始化
*               DEF_FAIL  初始化失败，可能是信号量
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_EEPROM_Init(void)
{
	OS_ERR      err;
    CPU_BOOLEAN status;
	
	OSSemCreate((OS_SEM    *)&SemLock,                                   //创建互斥信号量
                (CPU_CHAR  *)"EEPROM SemLock", 
	            (OS_SEM_CTR ) 1,
	            (OS_ERR    *)&err);
	
	if(OS_ERR_NONE != err) {
		return DEF_FAIL;
	}
  
	status = EEPROM_I2C_INIT( );
	
    return status;
}



/*
*********************************************************************************************************
*                                        BSP_EEPROM_Rd()
*
* Description : 从eerprom读取数据.
*
* Argument(s) : *ptr      读取缓冲区的头指针
*               len       表示要读取的数据长度
*               addr      eeprom的起始地址
*           
* Return(s)   : DEF_OK    读取数据成功
*               DEF_FAIL  未读取到制定长度的数据
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_EEPROM_Rd(CPU_INT08U *ptr, CPU_INT16U len, CPU_INT16U addr)
{
	OS_ERR      status;
    CPU_BOOLEAN err = DEF_FAIL;
    CPU_INT08U  devAddr = 0;
                                                                                     //检测参数有效性
    if(addr>EEPROM_ADDR_MAX || len>EEPROM_LEN_MAX || addr+len > EEPROM_LEN_MAX)                    
        return err;

	OSSemPend((OS_SEM *)&SemLock,                                                   //锁定eeprom
	          (OS_TICK ) EEPROM_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &status);
	if(OS_ERR_NONE != status)
		return DEF_FAIL;
	
	
    if(addr > 255) {                                                                 //超过了可寻址范围作为设备地址传输           
        devAddr = addr/256;
        devAddr <<= 1;
     }
	*ptr = addr%256;                                                                 //写入第一个字节为寻址范围内的地址
	
	err = EEPROM_I2C_WrRd(EEPROM_DEV_ADDR|devAddr,                                   //计算超过256的块地址
	                      ptr,
						  len);
	 
	OSSemPost((OS_SEM *)&SemLock,
	          (OS_OPT  )OS_OPT_POST_1,
	          (OS_ERR *) &status);
	 
	if(OS_ERR_NONE != status)
		return DEF_FAIL;
	 
    return err;
}



/*
*********************************************************************************************************
*                                        BSP_EEPROM_Wr()
*
* Description : 向eerprom指定地址写入数据.
*
* Argument(s) : *ptr      读取缓冲区的头指针
*               len       表示要读取的数据长度
*               addr      eeprom的起始地址
*           
* Return(s)   : DEF_OK    写入数据成功
*               DEF_FAIL  指定长度的数据写入失败
*
* Caller(s)   : Application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_EEPROM_Wr(CPU_INT08U *ptr, CPU_INT16U len, CPU_INT16U addr)
{
    CPU_BOOLEAN status = DEF_FAIL;
    OS_ERR      err;
    CPU_INT08U  devAddr = 0, index_b;
	CPU_INT16U  index_p = 0;
    CPU_INT08U  buff[PAGE_SIZE+1];                                                   //用于页写入缓存与存地址
    CPU_INT08U  firstMax;                                                            //第一次可能写入的最大字节数
                                                                                     //检测参数有效性
    if(addr>EEPROM_ADDR_MAX || len>EEPROM_LEN_MAX || addr+len > EEPROM_LEN_MAX)                    
        return status;

	OSSemPend((OS_SEM *)&SemLock,                                                    //锁定eeprom
	          (OS_TICK ) EEPROM_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
    if(addr > 255) {                                                                 //超过了可寻址范围作为设备地址传输           
        devAddr = addr/256;
        devAddr <<= 1;
    }
	
    buff[0] = addr%256;                                                              //可寻址范围内地址
    firstMax = PAGE_SIZE - (addr%PAGE_SIZE);                                         //填满当前页需要写的字节数
	
/*------------------------------  不需要夸页写  -----------------------------*/	
    if(len <= firstMax) {                                                            //写入字节长度未延伸到下一页
		for(index_b = 1; index_b <= len; index_b++, index_p++)                       //将要写入数据拷贝进缓存区
        {
            *(buff+index_b) = *(ptr+index_p);
        }
		
		status = EEPROM_I2C_Wr(EEPROM_DEV_ADDR|devAddr,                             
							   buff,
							   len+1);
		
		OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                 //等待一段时间
					   OS_OPT_TIME_HMSM_STRICT,
                       &err );
		
		OSSemPost((OS_SEM *) &SemLock,                                              //释放EEPROM
				  (OS_OPT  ) OS_OPT_POST_1,
				  (OS_ERR *) &err);
	 
		if(OS_ERR_NONE != err)
			return DEF_FAIL;
                         
        return status;
    }

/*---------------------  写页前面部分，使其页对齐  ---------------------------*/
    for(index_b = 1; index_b <= firstMax; index_b++, index_p++)                     //将要写入数据拷贝进缓存区
    {
        *(buff+index_b) = *(ptr+index_p);
    }
	
    status = EEPROM_I2C_Wr( EEPROM_DEV_ADDR|devAddr,                                //设备地址加上不可寻址地址                                 
							buff,
							firstMax+1);    
	                    
    len  -= firstMax;
	addr += firstMax;
    OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                    //等待一段时间
	               OS_OPT_TIME_HMSM_STRICT,
	               &err );

 /*----------------------------   开始写完整的页   -------------------------*/                  
	while(len >= PAGE_SIZE) {
       if(addr > 255) {                                                             //超过了可寻址范围作为设备地址传输           
           devAddr = addr/256;
           devAddr <<= 1;
       }
	   
       buff[0] = addr%256;                                                          //可寻址范围内地
       for(index_b = 1; index_b <= PAGE_SIZE; index_b++, index_p++)                 //将要写入数据拷贝进缓存区
       {
           *(buff+index_b) = *(ptr+index_p);
       }   
	   
       status = EEPROM_I2C_Wr(EEPROM_DEV_ADDR|devAddr,                                  
							  buff,
							  PAGE_SIZE+1);

       len  -= PAGE_SIZE;
       addr += PAGE_SIZE;
       OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                 //等待一段时间
                      OS_OPT_TIME_HMSM_STRICT,
                      &err );
     }

/*------------------------  剩余不足一页部分数据              -----------------*/
	if(len>0) {
		if(addr > 255) {                                                            //超过了可寻址范围作为设备地址传输           
			devAddr = addr/256;
            devAddr <<= 1;
        }
		
        buff[0] = addr%256;
        for(index_b = 1; index_b <= len; index_b++, index_p++)                      //将要写入数据拷贝进缓存区
        {
			*(buff+index_b) = *(ptr+index_p);
        }   
		
        status = EEPROM_I2C_Wr(EEPROM_DEV_ADDR|devAddr,                                  
							   buff,
							   len+1); 
		
		OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                //等待一段时间
                      OS_OPT_TIME_HMSM_STRICT,
                      &err );
    }
	
	OSSemPost((OS_SEM *) &SemLock,                                                  //释放EEPROM
			  (OS_OPT  ) OS_OPT_POST_1,
			  (OS_ERR *) &err);
	 
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	 
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
*                                        EEPROM_I2C_INIT()
*
* Description : 初始化底层的IIC端口
*
* Argument(s) : none
*           
* Return(s)   : DEF_OK    I2C初始化成功
*               DEF_FAIL  I2C初始化失败
*
* Caller(s)   : BSP_EEPROM_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  CPU_BOOLEAN  EEPROM_I2C_INIT (void)
{
	CPU_BOOLEAN err;
	
	err =BSP_I2C_Init(EEPROM_I2C_PORT, 
                      BSP_I2C_MODE_STANDARD, 
                      BSP_I2C_MODE_STANDARD_MAX_FREQ_HZ);
	
	return err;
}



/*
*********************************************************************************************************
*                                        EEPROM_I2C_WrRd()
*
* Description : 向地址p_buf[0]连续读取nbr_bytes个字节
*
* Argument(s) : i2c_addr  设备地址，也包含不能寻址的块地址
*
*               *p_buf    第一个字节存储块内地址，返回后存储读取到的数据
*
*               nbr_bytes 要读取的字节数
*           
* Return(s)   : DEF_OK    读取到制定个数字节
*               DEF_FAIL  未读取到制定个数字节
*
* Caller(s)   : BSP_EEPROM_Rd()
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  CPU_BOOLEAN  EEPROM_I2C_WrRd (CPU_INT08U   i2c_addr,
									  CPU_INT08U  *p_buf,
                                      CPU_INT16U   nbr_bytes)
{
	CPU_BOOLEAN err;
	
	err = BSP_I2C_WrRd(EEPROM_I2C_PORT,
					   i2c_addr,                                  
                       p_buf,
                       nbr_bytes);
	
	return err;
	
}


/*
*********************************************************************************************************
*                                        EEPROM_I2C_WrRd()
*
* Description : 连续向IIC总线写 nbr_bytes 个字节
*
* Argument(s) : i2c_addr  设备地址，也包含不能寻址的块地址
*
*               *p_buf    将要向IIC总线写得数据
*
*               nbr_bytes 要写的字节数
*           
* Return(s)   : DEF_OK    写出指定个数字节成功
*               DEF_FAIL  写出指定个数字节失败
*
* Caller(s)   : BSP_EEPROM_Wr()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  EEPROM_I2C_Wr (CPU_INT08U   i2c_addr,
								    CPU_INT08U  *p_buf,
								    CPU_INT16U   nbr_bytes)
{
	CPU_BOOLEAN err;
	
	err = BSP_I2C_Wr(EEPROM_I2C_PORT,                                       
                     i2c_addr,                                  
                     p_buf,
                     nbr_bytes);
	
	return err;
	
}



