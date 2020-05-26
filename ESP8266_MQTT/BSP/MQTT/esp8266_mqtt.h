#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx_hal.h"

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))
	
//MQTT连接服务器
extern uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password);
//MQTT消息订阅
extern uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether);
//MQTT消息发布
extern uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos);
//MQTT发送心跳包
extern void MQTT_SentHeart(void);

#endif
