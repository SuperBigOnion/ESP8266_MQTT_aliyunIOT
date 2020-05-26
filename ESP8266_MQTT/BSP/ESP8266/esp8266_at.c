#include "esp8266_at.h"

//usart1���ͺͽ�������
uint8_t usart1_txbuf[256];
uint8_t usart1_rxbuf[512];
uint8_t usart1_rxone[1];
uint8_t usart1_rxcounter;


//����1����һ���ֽ�
static void USART1_SendOneByte(uint8_t val)
{
	((UART_HandleTypeDef *)&huart1)->Instance->DR = ((uint16_t)val & (uint16_t)0x01FF);
	while((((UART_HandleTypeDef *)&huart1)->Instance->SR&0X40)==0);//�ȴ��������
}


//��ESP8266���Ͷ�������
void ESP8266_ATSendBuf(uint8_t* buf,uint16_t len)
{
	memset(usart1_rxbuf,0, 256);
	
	//ÿ�η���ǰ�����մ��ڽ���������0,Ϊ�˽���
	usart1_rxcounter = 0;	
	
	//��������
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,len,0xFFFF);
}

//��ESP8266�����ַ���
void ESP8266_ATSendString(char* str)
{
  memset(usart1_rxbuf,0, 256);
	
	//ÿ�η���ǰ�����մ��ڽ���������0,Ϊ�˽���
	usart1_rxcounter = 0;	
	
	//���ͷ���1
	while(*str)		USART1_SendOneByte(*str++);
	
	//���ͷ���2
//	HAL_UART_Transmit(&huart1,(uint8_t *)str,strlen(str),0xFFFF);
}

//�˳�͸��
void ESP8266_ExitUnvarnishedTrans(void)
{
	ESP8266_ATSendString("+++");HAL_Delay(50);
	ESP8266_ATSendString("+++");HAL_Delay(50);	
}

//�����ַ������Ƿ������һ���ַ���
uint8_t FindStr(char* dest,char* src,uint16_t retry_nms)
{
	retry_nms/=10;                   //��ʱʱ��

	while(strstr(dest,src)==0 && retry_nms--)//�ȴ����ڽ�����ϻ�ʱ�˳�
	{		
		HAL_Delay(10);
	}

	if(retry_nms) return 1;                       

	return 0; 
}

/**
 * ���ܣ����ESP8266�Ƿ�����
 * ������None
 * ����ֵ��ESP8266����״̬
 *        ��0 ESP8266����
 *        0 ESP8266������  
 */
uint8_t ESP8266_Check(void)
{
	uint8_t check_cnt=5;
	while(check_cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); 	 //��ս��ջ���
		ESP8266_ATSendString("AT\r\n");     		 			//����AT����ָ��	
		if(FindStr((char*)usart1_rxbuf,"OK",200) != 0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * ���ܣ���ʼ��ESP8266
 * ������None
 * ����ֵ����ʼ���������0Ϊ��ʼ���ɹ�,0Ϊʧ��
 */
uint8_t ESP8266_Init(void)
{
	//��շ��ͺͽ�������
	memset(usart1_txbuf,0,sizeof(usart1_txbuf));
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));
	
	ESP8266_ExitUnvarnishedTrans();		//�˳�͸��
	HAL_Delay(500);
	ESP8266_ATSendString("AT+RST\r\n");
	HAL_Delay(800);
	if(ESP8266_Check()==0)              //ʹ��ATָ����ESP8266�Ƿ����
	{
		return 0;
	}
	
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));    //��ս��ջ���
	ESP8266_ATSendString("ATE0\r\n");     	//�رջ��� 
	if(FindStr((char*)usart1_rxbuf,"OK",500)==0)  //���ò��ɹ�
	{
			return 0;      
	}
	return 1;                         //���óɹ�
}

/**
 * ���ܣ��ָ���������
 * ������None
 * ����ֵ��None
 * ˵��:��ʱESP8266�е��û����ý�ȫ����ʧ�ظ��ɳ���״̬
 */
void ESP8266_Restore(void)
{
	ESP8266_ExitUnvarnishedTrans();          	//�˳�͸��
  HAL_Delay(500);
	ESP8266_ATSendString("AT+RESTORE\r\n");		//�ָ����� 	
}

/**
 * ���ܣ������ȵ�
 * ������
 *         ssid:�ȵ���
 *         pwd:�ȵ�����
 * ����ֵ��
 *         ���ӽ��,��0���ӳɹ�,0����ʧ��
 * ˵���� 
 *         ʧ�ܵ�ԭ�������¼���(UARTͨ�ź�ESP8266���������)
 *         1. WIFI�������벻��ȷ
 *         2. ·���������豸̫��,δ�ܸ�ESP8266����IP
 */
uint8_t ESP8266_ConnectAP(char* ssid,char* pswd)
{
	uint8_t cnt=5;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));     
		ESP8266_ATSendString("AT+CWMODE_CUR=1\r\n");              //����ΪSTATIONģʽ	
		if(FindStr((char*)usart1_rxbuf,"OK",200) != 0)
		{
			break;
		}             		
	}
	if(cnt == 0)
		return 0;

	cnt=2;
	while(cnt--)
	{                    
		memset(usart1_txbuf,0,sizeof(usart1_txbuf));//��շ��ͻ���
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));//��ս��ջ���
		sprintf((char*)usart1_txbuf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",ssid,pswd);//����Ŀ��AP
		ESP8266_ATSendString((char*)usart1_txbuf);	
		if(FindStr((char*)usart1_rxbuf,"OK",8000)!=0)                      //���ӳɹ��ҷ��䵽IP
		{
			return 1;
		}
	}
	return 0;
}

//����͸��ģʽ
static uint8_t ESP8266_OpenTransmission(void)
{
	//����͸��ģʽ
	uint8_t cnt=2;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));    
		ESP8266_ATSendString("AT+CIPMODE=1\r\n");  
		if(FindStr((char*)usart1_rxbuf,"OK",200)!=0)
		{	
			return 1;
		}
	}
	return 0;
}

/**
 * ���ܣ�ʹ��ָ��Э��(TCP/UDP)���ӵ�������
 * ������
 *         mode:Э������ "TCP","UDP"
 *         ip:Ŀ�������IP
 *         port:Ŀ���Ƿ������˿ں�
 * ����ֵ��
 *         ���ӽ��,��0���ӳɹ�,0����ʧ��
 * ˵���� 
 *         ʧ�ܵ�ԭ�������¼���(UARTͨ�ź�ESP8266���������)
 *         1. Զ�̷�����IP�Ͷ˿ں�����
 *         2. δ����AP
 *         3. �������˽�ֹ���(һ�㲻�ᷢ��)
 */
uint8_t ESP8266_ConnectServer(char* mode,char* ip,uint16_t port)
{
	uint8_t cnt;
   
	ESP8266_ExitUnvarnishedTrans();                   //����������˳�͸��
	HAL_Delay(500);

	//���ӷ�����
	cnt=2;
	while(cnt--)
	{
		memset(usart1_txbuf,0,sizeof(usart1_txbuf));//��շ��ͻ���
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf));//��ս��ջ���   
		sprintf((char*)usart1_txbuf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",mode,ip,port);
		ESP8266_ATSendString((char*)usart1_txbuf);
		if(FindStr((char*)usart1_rxbuf,"CONNECT",8000) !=0 )
		{
			break;
		}
	}
	if(cnt == 0) 
		return 0;
	
	//����͸��ģʽ
	if(ESP8266_OpenTransmission()==0) return 0;
	
	//��������״̬
	cnt=2;
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //��ս��ջ���   
		ESP8266_ATSendString("AT+CIPSEND\r\n");//��ʼ����͸������״̬
		if(FindStr((char*)usart1_rxbuf,">",200)!=0)
		{
			return 1;
		}
	}
	return 0;
}

/**
 * ���ܣ������ͷ������Ͽ�����
 * ������None
 * ����ֵ��
 *         ���ӽ��,��0�Ͽ��ɹ�,0�Ͽ�ʧ��
 */
uint8_t DisconnectServer(void)
{
	uint8_t cnt;
	
	ESP8266_ExitUnvarnishedTrans();	//�˳�͸��
	HAL_Delay(500);
	
	while(cnt--)
	{
		memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //��ս��ջ���   
		ESP8266_ATSendString("AT+CIPCLOSE\r\n");//�ر�����

		if(FindStr((char*)usart1_rxbuf,"CLOSED",200)!=0)//�����ɹ�,�ͷ������ɹ��Ͽ�
		{
			break;
		}
	}
	if(cnt) return 1;
	return 0;
}
