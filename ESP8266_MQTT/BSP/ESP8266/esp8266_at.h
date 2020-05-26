#ifndef __ESP8266_AT_H
#define __ESP8266_AT_H

#include "stm32f4xx_hal.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

extern uint8_t usart1_txbuf[256];
extern uint8_t usart1_rxbuf[512];
extern uint8_t usart1_rxone[1];
extern uint8_t usart1_rxcounter;

extern uint8_t ESP8266_Init(void);
extern void ESP8266_Restore(void);

extern void ESP8266_ATSendBuf(uint8_t* buf,uint16_t len);		//向ESP8266发送指定长度数据
extern void ESP8266_ATSendString(char* str);								//向ESP8266模块发送字符串
extern void ESP8266_ExitUnvarnishedTrans(void);							//ESP8266退出透传模式
extern uint8_t ESP8266_ConnectAP(char* ssid,char* pswd);		//ESP8266连接热点
extern uint8_t ESP8266_ConnectServer(char* mode,char* ip,uint16_t port);	//使用指定协议(TCP/UDP)连接到服务器

#endif
