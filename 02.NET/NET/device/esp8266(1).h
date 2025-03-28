#ifndef _ESP8266_H_
#define _ESP8266_H_
/*-------------------------------------------------*/
/*                                                 */
/*              操作Wifi功能的头文件               */
/*                                                 */
/*-------------------------------------------------*/
#include "git.h"

#define REV_OK 0   // 接收完成标志
#define REV_WAIT 1 // 接收未完成标志

void ESP8266_Init(void);
void USART2_Init(unsigned int baud);
void ESP8266_Clear(void);

void ESP8266_SendData(unsigned char *data, unsigned short len);
void ESP8266_SendMqtt(unsigned char *data, unsigned short len);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);
void USART2_SendString(unsigned char *str, unsigned short len);

void Get_Time(void);

#endif
