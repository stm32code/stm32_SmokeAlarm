// 单片机头文件
#include "stm32f10x.h"
// 网络设备驱动
#include "esp8266.h"
// 硬件驱动
#include "delay.h"
#include "usart.h"
// C库
#include <string.h>
#include <stdio.h>

#define ESP8266_WIFI_INFO "AT+CWJAP=\"NET\",\"12345678\"\r\n"
// 苏宁后台获取时间的API
#define Time_TCP "AT+CIPSTART=\"TCP\",\"quan.suning.com\",80\r\n"
// 苏宁后台获取时间GET报文
#define Time_GET "GET https://quan.suning.com/getSysTime.do\r\n"
// 存放时间数据
unsigned char Time_buff[100]; // 位数是随机确定的

unsigned char esp8266_buf[256];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

//==========================================================
//	函数名称：	ESP8266_Clear
//
//	函数功能：	清空缓存
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Clear(void)
{

	memset(esp8266_buf, 0, sizeof(esp8266_buf));
	esp8266_cnt = 0;
}

//==========================================================
//	函数名称：	ESP8266_WaitRecive
//
//	函数功能：	等待接收完成
//
//	入口参数：	无
//
//	返回参数：	REV_OK-接收完成		REV_WAIT-接收超时未完成
//
//	说明：		循环调用检测是否接收完成
//==========================================================
_Bool ESP8266_WaitRecive(void)
{

	if (esp8266_cnt == 0) // 如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return REV_WAIT;

	if (esp8266_cnt == esp8266_cntPre) // 如果上一次的值和这次相同，则说明接收完毕
	{
		esp8266_cnt = 0; // 清0接收计数
		// printf("%s\n", esp8266_buf);
		return REV_OK; // 返回接收完成标志
	}

	esp8266_cntPre = esp8266_cnt; // 置为相同

	return REV_WAIT; // 返回接收未完成标志
}

//==========================================================
//	函数名称：	ESP8266_SendCmd
//
//	函数功能：	发送命令
//
//	入口参数：	cmd：命令
//				res：需要检查的返回指令
//
//	返回参数：	0-成功	1-失败
//
//	说明：
//==========================================================
_Bool ESP8266_SendCmd(char *cmd, char *res, u16 time)
{
	USART2_SendString((unsigned char *)cmd, strlen((const char *)cmd));

	while (time--)
	{
		if (ESP8266_WaitRecive() == REV_OK) // 如果收到数据
		{
			if (strstr((const char *)esp8266_buf, res) != NULL) // 如果检索到关键词
			{
				ESP8266_Clear(); // 清空缓存

				return 0;
			}
		}

		delay_ms(10);
	}

	return 1;
}

//==========================================================
//	函数名称：	ESP8266_SendData
//
//	函数功能：	发送数据
//
//	入口参数：	data：数据
//				len：长度
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	char cmdBuf[32];

	ESP8266_Clear();						   // 清空接收缓存
	sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len); // 发送命令
	if (!ESP8266_SendCmd(cmdBuf, ">", 200))	   // 收到‘>’时可以发送数据
	{

		USART2_SendString(data, len); // 发送设备连接请求数据
	}
}
void ESP8266_SendMqtt(unsigned char *data, unsigned short len)
{
	ESP8266_Clear();			  // 清空接收缓存
	USART2_SendString(data, len); // 发送设备连接请求数据
}
//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	获取平台返回的数据
//
//	入口参数：	等待的时间(乘以10ms)
//
//	返回参数：	平台返回的原始数据
//
//	说明：		不同网络设备返回的格式不同，需要去调试
//				如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
//==========================================================
unsigned char *ESP8266_GetIPD(unsigned short timeOut)
{
	char *ptrIPD = NULL;
	do
	{
		if (ESP8266_WaitRecive() == REV_OK) // 如果接收完成
		{
			ptrIPD = strstr((char *)esp8266_buf, "IPD,"); // 搜索“IPD”头
			if (ptrIPD == NULL)							  // 如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
			{
				// printf("\"IPD\" not found\r\n");
			}
			else
			{
				ptrIPD = strchr(ptrIPD, ':'); // 找到':'
				if (ptrIPD != NULL)
				{
					ptrIPD++;
					return (unsigned char *)(ptrIPD);
				}
				else
					return NULL;
			}
		}

		delay_ms(5); // 延时等待
	} while (timeOut--);

	return NULL; // 超时还未找到，返回空指针
}

//==========================================================
//	函数名称：	ESP8266_Init
//
//	函数功能：	初始化ESP8266
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void ESP8266_Init(void)
{
	char str[125];

	delay_ms(250);


	ESP8266_Clear();

	printf("AT\r\n");
	while (ESP8266_SendCmd("AT\r\n\r", "OK", 200))
		delay_ms(300);

	printf("CWMODE\r\n");
	while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 200))
		delay_ms(300);

	printf("AT+CWDHCP\r\n");
	while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200))
		delay_ms(300);

	printf("CWJAP\r\n");
	memset(str, 0, sizeof(str));
	sprintf(str, "AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASS);
	while (ESP8266_SendCmd(str, "GOT IP", 200))
		delay_ms(300);

	printf("CIPSTART\r\n");
	memset(str, 0, sizeof(str));
	sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ServerIP, ServerPort);
	while (ESP8266_SendCmd(str, "CONNECT", 200))
		delay_ms(500);
	printf("ESP8266 Init OK\r\n");
	// Sys_Restart();//软件复位
}

//==========================================================
//	函数名称：	ESP8266_GetIPD
//
//	函数功能：	copy天气数据到buff数组里面
//
//	返回参数：	平台返回的原始数据
//
//	说明：		copy天气数据到buff
//==========================================================

unsigned char *ESP8266_GetIPD_GET(unsigned short timeOut, u8 *buff) // 这里我用了一个全局变量将esp8266buf储存到这个全局变量里面
{
	do
	{
		delay_ms(5);
	} while (timeOut--);

	strcpy((char *)buff, (char *)esp8266_buf);
	return buff;
}

void Get_Time(void)
{
	char str[125];

	ESP8266_Clear();

	printf("AT\r\n");
	while (ESP8266_SendCmd("AT\r\n\r", "OK", 200))
		delay_ms(300);

	while (ESP8266_SendCmd("+++", "", 250))
		delay_ms(500);

	while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK", 200))
		delay_ms(300);

	while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK", 200))
		delay_ms(300);

	memset(str, 0, sizeof(str));
	sprintf(str, "%s", ESP8266_WIFI_INFO);
	while (ESP8266_SendCmd(str, "GOT IP", 200))
		delay_ms(300);

	memset(str, 0, sizeof(str));
	sprintf(str, "%s", Time_TCP);
	while (ESP8266_SendCmd(str, "CONNECT", 200))
		delay_ms(500);

	// printf("6.访问苏宁服务器成功 OK\r\n");

	// while (ESP8266_SendCmd("AT+CIPMODE=1\r\n", "OK", 250)) // 开启透传模式
	// 	delay_ms(300);
	// printf("7.开启透传模式成功 OK\r\n");

	ESP8266_SendData((u8 *)Time_GET, sizeof(Time_GET)); // 发送AT+CIPSEND  以及 Time_GET

	ESP8266_GetIPD_GET(200, Time_buff); // 将串口数据取出来

	printf("%s\n", Time_buff);

	ESP8266_Clear(); // 清除缓存数据

	while (ESP8266_SendCmd("+++", "", 250))
		delay_ms(500);

	printf("9.退出透传模式成功 OK\r\n");
}
/*
************************************************************
*	函数名称：	USART2_Init
*
*	函数功能：	串口3初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PB10		RX-PB11
************************************************************
*/
void USART2_Init(unsigned int baud)
{

	GPIO_InitTypeDef gpio_initstruct;
	USART_InitTypeDef usart_initstruct;
	NVIC_InitTypeDef nvic_initstruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// PA2	TXD
	gpio_initstruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_2;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_initstruct);

	// PA3	RXD
	gpio_initstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_initstruct.GPIO_Pin = GPIO_Pin_3;
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_initstruct);

	usart_initstruct.USART_BaudRate = baud;
	usart_initstruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	usart_initstruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				 // 接收和发送
	usart_initstruct.USART_Parity = USART_Parity_No;							 // 无校验
	usart_initstruct.USART_StopBits = USART_StopBits_1;							 // 1位停止位
	usart_initstruct.USART_WordLength = USART_WordLength_8b;					 // 8位数据位
	USART_Init(USART2, &usart_initstruct);

	USART_Cmd(USART2, ENABLE); // 使能串口

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 使能接收中断

	nvic_initstruct.NVIC_IRQChannel = USART2_IRQn;
	nvic_initstruct.NVIC_IRQChannelCmd = ENABLE;
	nvic_initstruct.NVIC_IRQChannelPreemptionPriority = 1;
	nvic_initstruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&nvic_initstruct);
}

//==========================================================
//	函数名称：	USART2_IRQHandler
//
//	函数功能：	串口2收发中断
//
//	入口参数：	无
//
//	返回参数：	无
//
//	说明：
//==========================================================
void USART2_IRQHandler(void)
{

	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) // 接收中断
	{
		if (esp8266_cnt >= sizeof(esp8266_buf))
			esp8266_cnt = 0; // 防止串口被刷爆
		esp8266_buf[esp8266_cnt++] = USART2->DR;

		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}
}

/*
************************************************************
*	函数名称：	Usart_SendString
*
*	函数功能：	串口数据发送
*
*	入口参数：	USARTx：串口组
*				str：要发送的数据
*				len：数据长度
*
*	返回参数：	无
*
*	说明：
************************************************************
*/
void USART2_SendString(unsigned char *str, unsigned short len)
{

	unsigned short count = 0;
	// printf("%s\n", str);
	for (; count < len; count++)
	{
		USART_SendData(USART2, *str++); // 发送数据
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
			; // 等待发送完成
	}
}
