/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hal_temp_hum.h"
#include "esp8266_at.h"
#include "esp8266_mqtt.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define USER_MAIN_DEBUG

#ifdef USER_MAIN_DEBUG
#define user_main_printf(format, ...) printf( format "\r\n",##__VA_ARGS__)
#define user_main_info(format, ...) printf("��main��info:" format "\r\n",##__VA_ARGS__)
#define user_main_debug(format, ...) printf("��main��debug:" format "\r\n",##__VA_ARGS__)
#define user_main_error(format, ...) printf("��main��error:" format "\r\n",##__VA_ARGS__)
#else
#define user_main_printf(format, ...)
#define user_main_info(format, ...)
#define user_main_debug(format, ...)
#define user_main_error(format, ...)
#endif

//�˴������Լ���wifi������
#define WIFI_NAME "HappyOneDay"
#define WIFI_PASSWD "1234567890"

//�˴��ǰ����Ʒ������ĵ�½����
#define MQTT_BROKERADDRESS "a1lAoazdH1w.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_CLIENTID "00001|securemode=3,signmethod=hmacsha1|"
#define MQTT_USARNAME "BZL01&a1lAoazdH1w"
#define MQTT_PASSWD "51A5BB10306E976D6C980F73037F2D9496D2813A"
#define	MQTT_PUBLISH_TOPIC "/sys/a1lAoazdH1w/BZL01/thing/event/property/post"
#define MQTT_SUBSCRIBE_TOPIC "/sys/a1lAoazdH1w/BZL01/thing/service/property/set"

//�˴�����ѭ��������ʱ�궨��
#define LOOPTIME 30 	//��������ѭ����ʱʱ�䣺30ms
#define COUNTER_LEDBLINK			(300/LOOPTIME)		//LED������˸ʱ�䣺300ms
#define COUNTER_RUNINFOSEND		(5000/LOOPTIME)		//���д�����ʾ��5s
#define COUNTER_MQTTHEART     (5000/LOOPTIME)		//MQTT������������5s
#define COUNTER_STATUSREPORT	(3000/LOOPTIME)		//״̬�ϴ���3s

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char mqtt_message[300];	//MQTT���ϱ���Ϣ����
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Enter_ErrorMode(uint8_t mode);
void ES8266_MQTT_Init(void);
void Change_LED_Status(void);
void STM32DHT11_StatusReport(void);
void deal_MQTT_message(uint8_t* buf,uint16_t len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	//����USART1�����ж�
	HAL_UART_Receive_IT(&huart1,usart1_rxone,1);			//��USART1�жϣ����ն�����Ϣ
	ES8266_MQTT_Init();																			//��ʼ��MQTT
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint16_t Counter_RUNLED_Blink = 0;
	uint16_t Counter_RUNInfo_Send = 0;
	uint16_t Counter_MQTT_Heart = 0;
	uint16_t Counter_StatusReport = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//���е���˸
//		if(Counter_RUNLED_Blink++>COUNTER_LEDBLINK)
//		{
//			Counter_RUNLED_Blink = 0;
//			HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
//		}
		
		//����״̬��ӡ
		if(Counter_RUNInfo_Send++>COUNTER_RUNINFOSEND)
		{
			Counter_RUNInfo_Send = 0;
			user_main_info("�����������У�\r\n");
		}
		
		//����������
//		if(Counter_MQTT_Heart++>COUNTER_MQTTHEART)
//		{
//			Counter_MQTT_Heart = 0;
//			MQTT_SentHeart();
//		}
		
		//����״̬�ϱ�
		if(Counter_StatusReport++>COUNTER_STATUSREPORT)
		{
			Counter_StatusReport = 0;
			STM32DHT11_StatusReport();
		}
		
		//������ջ���������
		if(usart1_rxcounter)
		{
			deal_MQTT_message(usart1_rxbuf,usart1_rxcounter);
		}

		HAL_Delay(LOOPTIME);
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/******************************  USART1�����жϴ���  *****************************/

// ES8266�������ڽ����жϴ�����
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)	// �ж������ĸ����ڴ������ж�
	{
		//�����յ������ݷ������usart1��������
		usart1_rxbuf[usart1_rxcounter] = usart1_rxone[0];
		usart1_rxcounter++;	//����������1
		
		//����ʹ�ܴ���1�����ж�
		HAL_UART_Receive_IT(&huart1,usart1_rxone,1);		
	}
}

/******************************  �����жϴ���  *****************************/

//KEY1���¶���ִ�к���
void KEY1_Pressed(void)
{
	user_main_debug("�Ұ�����KEY_1\r\n");
	Change_LED_Status();
}

//KEY2���¶���ִ�к���
void KEY2_Pressed(void)
{
	user_main_debug("�Ұ�����KEY_2\r\n");
}

//�����жϴ�����
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)  
    {  
        case KEY_1_Pin:KEY1_Pressed();break;  
        case KEY_2_Pin:KEY2_Pressed();break;   
        default:break;  
    }  
}

//�ı�LED��״̬
void Change_LED_Status(void)
{
	static uint8_t ledstatus = 0;
	
	switch(ledstatus++)
	{
		case 0://000
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);	
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
		break;
		case 1://001
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		break;
		case 2://010
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
		break;
		case 3://011
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		break;
		case 4://100
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
		break;
		case 5://101
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		break;
		case 6://110
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
		break;
		case 7://111
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		break;
		default:ledstatus=0;break;
	}
}

/******************************  �������ģʽ����  *****************************/

//�������ģʽ�ȴ��ֶ�����
void Enter_ErrorMode(uint8_t mode)
{
	HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
	while(1)
	{
		switch(mode){
			case 0:user_main_error("ESP8266��ʼ��ʧ�ܣ�\r\n");break;
			case 1:user_main_error("ESP8266�����ȵ�ʧ�ܣ�\r\n");break;
			case 2:user_main_error("ESP8266���Ӱ����Ʒ�����ʧ�ܣ�\r\n");break;
			case 3:user_main_error("ESP8266������MQTT��½ʧ�ܣ�\r\n");break;
			case 4:user_main_error("ESP8266������MQTT��������ʧ�ܣ�\r\n");break;
			default:user_main_info("Nothing\r\n");break;
		}
		user_main_info("������������");
		//HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		HAL_Delay(200);
	}
}

/******************************  ������������Դ���  *****************************/

/* ���Դ��ڲ��Ժ��� */
void TEST_usart2(void)
{
	user_main_debug("����USART2���Դ��룡\n");
	HAL_Delay(1000);
}

/* LED���Ժ��� */
void TEST_LED(void)
{
	//�������������
	HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
	HAL_Delay(500);
	//�̵�����������
	HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
	HAL_Delay(500);
	//��������������
	HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
	HAL_Delay(500);
}

/* us������ʱ���Ժ��� */
void TEST_delayus(void)
{
	static uint16_t tim1_test;
	
	//��ʱ1000��1ms
	for(tim1_test = 0;tim1_test<1000;tim1_test++)
	{
		//��ʱ1ms
		TIM1_Delay_us(1000);
	}
	user_main_debug("����us����ʱ�������Դ��룬1s��ӡһ�Σ�\n");
}

/* DHT11��ʪ�ȴ��������Ժ��� */
void TEST_DHT11(void)
{
	uint8_t temperature;
	uint8_t humidity;
	uint8_t get_times;

	// ��ȡ��ʪ����Ϣ���ô��ڴ�ӡ,��ȡʮ�Σ�ֱ���ɹ�����
	for(get_times=0;get_times<10;get_times++)
	{
		if(!dht11Read(&temperature, &humidity))//Read DHT11 Value
		{
				user_main_info("temperature=%d,humidity=%d \n",temperature,humidity);
				break;
		}
	}
}

/* ESP8266&MQTT���Դ��� */
void TEST_ES8266MQTT(void)
{
	uint8_t status=0;

	//��ʼ��
	if(ESP8266_Init())
	{
		user_main_info("ESP8266��ʼ���ɹ���\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//�����ȵ�
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			user_main_info("ESP8266�����ȵ�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//���Ӱ�����IOT������
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			user_main_info("ESP8266���Ӱ����Ʒ������ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//��½MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			user_main_info("ESP8266������MQTT��½�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//��������
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			user_main_info("ESP8266������MQTT��������ɹ���\r\n");
		}
		else Enter_ErrorMode(4);
	}
}

/******************************  STM32 MQTTҵ�����  *****************************/

//MQTT��ʼ������
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;

	//��ʼ��
	if(ESP8266_Init())
	{
		user_main_info("ESP8266��ʼ���ɹ���\r\n");
		status++;
	}
	else Enter_ErrorMode(0);

	//�����ȵ�
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			user_main_info("ESP8266�����ȵ�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//���Ӱ�����IOT������
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			user_main_info("ESP8266���Ӱ����Ʒ������ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//��½MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			user_main_info("ESP8266������MQTT��½�ɹ���\r\n");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//��������
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			user_main_info("ESP8266������MQTT��������ɹ���\r\n");
		}
		else Enter_ErrorMode(4);
	}
}

//��Ƭ��״̬�ϱ�
void STM32DHT11_StatusReport(void)
{
	//��ȡ��ʪ����Ϣ
	uint8_t temperature;
	uint8_t humidity;
	uint8_t get_times;
	
	// ��ȡ��ʪ����Ϣ���ô��ڴ�ӡ,��ȡʮ�Σ�ֱ���ɹ�����
	for(get_times=0;get_times<10;get_times++)
	{
		if(!dht11Read(&temperature, &humidity))//Read DHT11 Value
		{
			user_main_info("temperature=%d,humidity=%d \n",temperature,humidity);
			break;
		}
	}
	
	//�ϱ�һ������
	uint8_t led_r_status = HAL_GPIO_ReadPin(LED_R_GPIO_Port,LED_R_Pin) ? 0:1;
	uint8_t led_g_status = HAL_GPIO_ReadPin(LED_G_GPIO_Port,LED_G_Pin) ? 0:1;
	uint8_t led_b_status = HAL_GPIO_ReadPin(LED_B_GPIO_Port,LED_B_Pin) ? 0:1;
	sprintf(mqtt_message,
	"{\"method\":\"thing.service.property.set\",\"id\":\"181454577\",\"params\":{\
		\"DHT11_Temperature\":%.1f,\
		\"DHT11_Humidity\":%.1f,\
		\"Switch_LEDR\":%d,\
		\"Switch_LEDG\":%d,\
		\"Switch_LEDB\":%d\
	},\"version\":\"1.0.0\"}",
	(float)temperature,
	(float)humidity,
	led_r_status,
	led_g_status,
	led_b_status
	);

	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
}

char temp_str[30];    // ��ʱ�Ӵ�
void ReadStrUnit(char * str,char *temp_str,int idx,int len)  // ��ĸ���л�ȡ���Ӵ�������ȵ���ʱ�Ӵ�
{
    int index;
    for(index = 0; index < len; index++)
    {
        temp_str[index] = str[idx+index];
    }
    temp_str[index] = '\0';
}
int GetSubStrPos(char *str1,char *str2)
{
    int idx = 0;
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if( len1 < len2)
    {
        //printf("error 1 \n"); // �Ӵ���ĸ����
        return -1;
    }

    while(1)
    {
        ReadStrUnit(str1,temp_str,idx,len2);    // ���ϻ�ȡ�Ĵ� ĸ���� idx λ�ô�������ʱ�Ӵ�
        if(strcmp(str2,temp_str)==0)break;      // ����ʱ�Ӵ����Ӵ�һ�£�����ѭ��
        idx++;                                  // �ı��ĸ����ȡ��ʱ�Ӵ���λ��
        if(idx>=len1)return -1;                 // �� idx �Ѿ�����ĸ�����ȣ�˵��ĸ�����������Ӵ�
    }

    return idx;    // �����Ӵ���һ���ַ���ĸ���е�λ��
}

//����MQTT�·�����Ϣ
void deal_MQTT_message(uint8_t* buf,uint16_t len)
{
	uint8_t data[512];
	uint16_t data_len = len;
	for(int i=0;i<data_len;i++)
	{
		data[i] = buf[i];
		HAL_UART_Transmit(&huart2,&data[i],1,100);
	}
	memset(usart1_rxbuf,0,sizeof(usart1_rxbuf)); //��ս��ջ���  
	usart1_rxcounter=0;
	//user_main_info("MQTT�յ���Ϣ,���ݳ���=%d \n",data_len);
	
	//�����Ƿ������ú��
	int i = GetSubStrPos((char*)data,"LEDR");
	if( i>0 )
	{
		uint8_t ledr_status = data[i+6]-'0';
		HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		if(ledr_status)
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
	}
	
	//�����Ƿ��������̵�
	i = GetSubStrPos((char*)data,"LEDG");
	if( i>0 )
	{
		uint8_t ledr_status = data[i+6]-'0';
		HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		if(ledr_status)
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
	}
	
	//�����Ƿ�����������
	i = GetSubStrPos((char*)data,"LEDB");
	if( i>0 )
	{
		uint8_t ledr_status = data[i+6]-'0';
		HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
		if(ledr_status)
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
	}

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
