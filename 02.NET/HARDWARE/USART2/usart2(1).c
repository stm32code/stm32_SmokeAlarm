#include "delay.h"
#include "usart2.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "git.h"
char rxdatabufer;
u16 point1 = 0;

_SaveData Save_Data;

#if EN_USART2_RX // 如果使能了接收
// 串口1中断服务程序
// 注意,读取USARTx->SR能避免莫名其妙的错误
char USART_RX2_BUF[USART2_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.
// 接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目
u16 USART2_RX_STA = 0; // 接收状态标记

void Usart2_Init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // 串口3时钟使能

	USART_DeInit(USART2); // 复位串口3

	// USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);			// 初始化PB10

	// USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  // 初始化PB11

	USART_InitStructure.USART_BaudRate = bound;										// 波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式

	USART_Init(USART2, &USART_InitStructure); // 初始化串口	3

	USART_Cmd(USART2, ENABLE); // 使能串口

	// 使能接收中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 开启中断

	// 设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	CLR_Buf(); // 清空缓存
}

void USART2_IRQHandler(void) // 串口1中断服务程序
{
	u8 Res;
#ifdef OS_TICKS_PER_SEC // 如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART2); //(USART1->DR);	//读取接收到的数据

		if (Res == '$')
		{
			point1 = 0;
		}

		USART_RX2_BUF[point1++] = Res;

		if (USART_RX2_BUF[0] == '$' && USART_RX2_BUF[4] == 'M' && USART_RX2_BUF[5] == 'C') // 确定是否收到"GPRMC/GNRMC"这一帧数据
		{
			if (Res == '\n')
			{
				memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);	 // 清空
				memcpy(Save_Data.GPS_Buffer, USART_RX2_BUF, point1); // 保存数据
				Save_Data.isGetData = true;
				point1 = 0;
				memset(USART_RX2_BUF, 0, USART2_REC_LEN); // 清空
			}
		}

		if (point1 >= USART2_REC_LEN)
		{
			point1 = USART2_REC_LEN;
		}
	}
#ifdef OS_TICKS_PER_SEC // 如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();
#endif
}

u8 Hand(char *a) // 串口命令识别函数
{
	if (strstr(USART_RX2_BUF, a) != NULL)
		return 1;
	else
		return 0;
}

void CLR_Buf(void) // 串口缓存清理
{
	memset(USART_RX2_BUF, 0, USART2_REC_LEN); // 清空
	point1 = 0;
}

void clrStruct()
{
	Save_Data.isGetData = false;
	Save_Data.isParseData = false;
	Save_Data.isUsefull = false;
	memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length); // 清空
	memset(Save_Data.UTCTime, 0, UTCTime_Length);
	memset(Save_Data.latitude, 0, latitude_Length);
	memset(Save_Data.N_S, 0, N_S_Length);
	memset(Save_Data.longitude, 0, longitude_Length);
	memset(Save_Data.E_W, 0, E_W_Length);
}

#endif
