#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "sys.h"
#define USART_REC_LEN 200 // 定义最大接收字节数 200
#define EN_USART1_RX 1    // 使能（1）/禁止（0）串口1接收

extern u8 USART_RX_BUF[USART_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符
extern u16 USART_RX_STA;               // 接收状态标记
// 如果想串口中断接收，请不要注释以下宏定义
void Usart1_Init(u32 bound);
/*********************************************************************************
* @Function	:  发送一个字节
* @Input		:  pUSARTx 串口名
                             ch 数据
* @Output		:
* @Return		:  None
* @Others		:  None
* @Date			:  2022-07-30
**********************************************************************************/
void Usart_SendByte(USART_TypeDef *pUSARTx, uint8_t ch);
/*********************************************************************************
* @Function	:  发送字符串
* @Input		:  pUSARTx 串口名
                             str 字符串
* @Output		:  None
* @Return		:  None
* @Others		:  None
* @Date			:  2022-07-30
**********************************************************************************/
void Usart_SendString(USART_TypeDef *pUSARTx, char *str);
/*********************************************************************************
* @Function	:  发送一个16位数
* @Input		:  pUSARTx 串口名
                             ch 16位数
* @Output		:  None
* @Return		:  None
* @Others		:  None
* @Date			:  2022-07-30
**********************************************************************************/
void Usart_SendHalfWord(USART_TypeDef *pUSARTx, uint16_t ch);
/*********************************************************************************
* @Function	:  发送8位的数组
* @Input		:  pUSARTx 串口名
                             array 数组
                             num 数组长度
* @Output		:  None
* @Return		:  None
* @Others		:  None
* @Date			:  2022-07-30
**********************************************************************************/
void Usart_SendArray(USART_TypeDef *pUSARTx, char *array, uint16_t num);

#endif
