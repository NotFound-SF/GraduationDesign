
/*
*********************************************************************************************************
*                           �ó������ڶ�дeeprom������AT24C 01 02 04 08 16ϵ�У�
*   ���ݲ�ͬ��errpromֻ��Ҫ�޸�bsp_eeprom.h�к궨���ͺ����� #define  AT_24C16������24C16��ֵ��ע�����
*   ��ͬ�ͺŵ��豸��ַʹ�õ�Axʹ�ò�ͬ�������ڶ����豸��ַEEPROM_DEV_ADDRʱ���뱣֤Ϊʹ�õ�λΪ0������Ѱ
*   ַ���ص���
*   ���ļ�ֻ�г�ʼ����������д����������ֲʱֻ��Ҫ�޸� EEPROM_I2C_INIT��EEPROM_I2C_WrRd��EEPROM_I2C_Wr
*   ���������I2C���
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

static     OS_SEM                 SemLock;                     //ȷ��eeprom������Ķ�д

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
* Return(s)   : DEF_OK    ������ʼ��
*               DEF_FAIL  ��ʼ��ʧ�ܣ��������ź���
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
	
	OSSemCreate((OS_SEM    *)&SemLock,                                   //���������ź���
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
* Description : ��eerprom��ȡ����.
*
* Argument(s) : *ptr      ��ȡ��������ͷָ��
*               len       ��ʾҪ��ȡ�����ݳ���
*               addr      eeprom����ʼ��ַ
*           
* Return(s)   : DEF_OK    ��ȡ���ݳɹ�
*               DEF_FAIL  δ��ȡ���ƶ����ȵ�����
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
                                                                                     //��������Ч��
    if(addr>EEPROM_ADDR_MAX || len>EEPROM_LEN_MAX || addr+len > EEPROM_LEN_MAX)                    
        return err;

	OSSemPend((OS_SEM *)&SemLock,                                                   //����eeprom
	          (OS_TICK ) EEPROM_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &status);
	if(OS_ERR_NONE != status)
		return DEF_FAIL;
	
	
    if(addr > 255) {                                                                 //�����˿�Ѱַ��Χ��Ϊ�豸��ַ����           
        devAddr = addr/256;
        devAddr <<= 1;
     }
	*ptr = addr%256;                                                                 //д���һ���ֽ�ΪѰַ��Χ�ڵĵ�ַ
	
	err = EEPROM_I2C_WrRd(EEPROM_DEV_ADDR|devAddr,                                   //���㳬��256�Ŀ��ַ
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
* Description : ��eerpromָ����ַд������.
*
* Argument(s) : *ptr      ��ȡ��������ͷָ��
*               len       ��ʾҪ��ȡ�����ݳ���
*               addr      eeprom����ʼ��ַ
*           
* Return(s)   : DEF_OK    д�����ݳɹ�
*               DEF_FAIL  ָ�����ȵ�����д��ʧ��
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
    CPU_INT08U  buff[PAGE_SIZE+1];                                                   //����ҳд�뻺������ַ
    CPU_INT08U  firstMax;                                                            //��һ�ο���д�������ֽ���
                                                                                     //��������Ч��
    if(addr>EEPROM_ADDR_MAX || len>EEPROM_LEN_MAX || addr+len > EEPROM_LEN_MAX)                    
        return status;

	OSSemPend((OS_SEM *)&SemLock,                                                    //����eeprom
	          (OS_TICK ) EEPROM_LOCK_TIME,
	          (OS_OPT  ) OS_OPT_PEND_BLOCKING,
	          (CPU_TS *) 0,
	          (OS_ERR *) &err);
	
	if(OS_ERR_NONE != err)
		return DEF_FAIL;
	
    if(addr > 255) {                                                                 //�����˿�Ѱַ��Χ��Ϊ�豸��ַ����           
        devAddr = addr/256;
        devAddr <<= 1;
    }
	
    buff[0] = addr%256;                                                              //��Ѱַ��Χ�ڵ�ַ
    firstMax = PAGE_SIZE - (addr%PAGE_SIZE);                                         //������ǰҳ��Ҫд���ֽ���
	
/*------------------------------  ����Ҫ��ҳд  -----------------------------*/	
    if(len <= firstMax) {                                                            //д���ֽڳ���δ���쵽��һҳ
		for(index_b = 1; index_b <= len; index_b++, index_p++)                       //��Ҫд�����ݿ�����������
        {
            *(buff+index_b) = *(ptr+index_p);
        }
		
		status = EEPROM_I2C_Wr(EEPROM_DEV_ADDR|devAddr,                             
							   buff,
							   len+1);
		
		OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                 //�ȴ�һ��ʱ��
					   OS_OPT_TIME_HMSM_STRICT,
                       &err );
		
		OSSemPost((OS_SEM *) &SemLock,                                              //�ͷ�EEPROM
				  (OS_OPT  ) OS_OPT_POST_1,
				  (OS_ERR *) &err);
	 
		if(OS_ERR_NONE != err)
			return DEF_FAIL;
                         
        return status;
    }

/*---------------------  дҳǰ�沿�֣�ʹ��ҳ����  ---------------------------*/
    for(index_b = 1; index_b <= firstMax; index_b++, index_p++)                     //��Ҫд�����ݿ�����������
    {
        *(buff+index_b) = *(ptr+index_p);
    }
	
    status = EEPROM_I2C_Wr( EEPROM_DEV_ADDR|devAddr,                                //�豸��ַ���ϲ���Ѱַ��ַ                                 
							buff,
							firstMax+1);    
	                    
    len  -= firstMax;
	addr += firstMax;
    OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                    //�ȴ�һ��ʱ��
	               OS_OPT_TIME_HMSM_STRICT,
	               &err );

 /*----------------------------   ��ʼд������ҳ   -------------------------*/                  
	while(len >= PAGE_SIZE) {
       if(addr > 255) {                                                             //�����˿�Ѱַ��Χ��Ϊ�豸��ַ����           
           devAddr = addr/256;
           devAddr <<= 1;
       }
	   
       buff[0] = addr%256;                                                          //��Ѱַ��Χ�ڵ�
       for(index_b = 1; index_b <= PAGE_SIZE; index_b++, index_p++)                 //��Ҫд�����ݿ�����������
       {
           *(buff+index_b) = *(ptr+index_p);
       }   
	   
       status = EEPROM_I2C_Wr(EEPROM_DEV_ADDR|devAddr,                                  
							  buff,
							  PAGE_SIZE+1);

       len  -= PAGE_SIZE;
       addr += PAGE_SIZE;
       OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                 //�ȴ�һ��ʱ��
                      OS_OPT_TIME_HMSM_STRICT,
                      &err );
     }

/*------------------------  ʣ�಻��һҳ��������              -----------------*/
	if(len>0) {
		if(addr > 255) {                                                            //�����˿�Ѱַ��Χ��Ϊ�豸��ַ����           
			devAddr = addr/256;
            devAddr <<= 1;
        }
		
        buff[0] = addr%256;
        for(index_b = 1; index_b <= len; index_b++, index_p++)                      //��Ҫд�����ݿ�����������
        {
			*(buff+index_b) = *(ptr+index_p);
        }   
		
        status = EEPROM_I2C_Wr(EEPROM_DEV_ADDR|devAddr,                                  
							   buff,
							   len+1); 
		
		OSTimeDlyHMSM( 0, 0, 0, EEPROM_WRITE_DLY_MS,                                //�ȴ�һ��ʱ��
                      OS_OPT_TIME_HMSM_STRICT,
                      &err );
    }
	
	OSSemPost((OS_SEM *) &SemLock,                                                  //�ͷ�EEPROM
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
* Description : ��ʼ���ײ��IIC�˿�
*
* Argument(s) : none
*           
* Return(s)   : DEF_OK    I2C��ʼ���ɹ�
*               DEF_FAIL  I2C��ʼ��ʧ��
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
* Description : ���ַp_buf[0]������ȡnbr_bytes���ֽ�
*
* Argument(s) : i2c_addr  �豸��ַ��Ҳ��������Ѱַ�Ŀ��ַ
*
*               *p_buf    ��һ���ֽڴ洢���ڵ�ַ�����غ�洢��ȡ��������
*
*               nbr_bytes Ҫ��ȡ���ֽ���
*           
* Return(s)   : DEF_OK    ��ȡ���ƶ������ֽ�
*               DEF_FAIL  δ��ȡ���ƶ������ֽ�
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
* Description : ������IIC����д nbr_bytes ���ֽ�
*
* Argument(s) : i2c_addr  �豸��ַ��Ҳ��������Ѱַ�Ŀ��ַ
*
*               *p_buf    ��Ҫ��IIC����д������
*
*               nbr_bytes Ҫд���ֽ���
*           
* Return(s)   : DEF_OK    д��ָ�������ֽڳɹ�
*               DEF_FAIL  д��ָ�������ֽ�ʧ��
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



