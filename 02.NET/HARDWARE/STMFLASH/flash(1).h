#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "stm32f10x_flash.h"
#include "git.h"

extern U32 addrShort1; // 1
extern U32 addrShort2; // 2


// 下面这三个定义，用于存取内部FLASH数据，F103系列都不用修改
#ifdef STM32F10X_MD
#define STM32_FLASH_SECTOR_SIZE 2048 // 内部FLASH页大小, 单位：bytes 　(注：STM32F10xx系列下，小中容量存储器扇区为1K, 大容量存储器扇区为2K）
#else
#define STM32_FLASH_SECTOR_SIZE 1024             　　　 // 内部FLASH页大小, 单位：bytes 　(注：STM32F10xx系列下，小中容量存储器扇区为1K, 大容量存储器扇区为2K）
#endif

#define STM32_FLASH_ADDR_BASE 0x08000000                  // 芯片内部FLASH基址(这个基本不用修改）
static uint8_t sectorbufferTemp[STM32_FLASH_SECTOR_SIZE]; // 开辟一段内存空间，作用：在内部FLASH写入时作数据缓冲

/*********************************************************************************
 * @Function	:  在芯片的内部FLASH里，写入指定长度数据
 * @Input		:  writeAddr,写入数据的地址
 *							 *writeToBuffer,写入数据
 *							 numToWrite,写入数据长度
 * @Output		:  None
 * @Return		:  0_成功，
 *         		 1_失败，地址范围不正确
 *         		 2_失败，FLASH->SR:BSY忙超时
 *         		 3_失败，擦除超时
 * @Others		:  None
 * @Date			:  2022-08-23
 **********************************************************************************/
uint8_t System_WriteInteriorFlash(uint32_t writeAddr, uint8_t *writeToBuffer, uint16_t numToWrite);
/*********************************************************************************
 * @Function	:  在芯片的内部FLASH里，读取指定长度数据
 * @Input		:  writeAddr,写入数据的地址
 *							 *writeToBuffer,写入数据
 *							 numToWrite,写入数据长度
 * @Output		:  None
 * @Return		:  0_成功
 *         		 1_失败，地址小于FLASH基址
 *         		 2_失败，地址大于FLASH最大值
 * @Others		:  None
 * @Date			:  2022-08-23
 **********************************************************************************/
uint8_t System_ReadInteriorFlash(uint32_t readAddr, uint8_t *pBuffer, uint16_t numToRead);
/*********************************************************************************
 * @Function	:  测试FALSH读写程序
 * @Input		:  None
 * @Output		:  None
 * @Return		:  None
 * @Others		:  演示程序的几种数据读写方法
 * @Date			:  2022-08-23
 **********************************************************************************/
void W_R_Test(void);
void W_Test(void);
// 读取数据
void R_Test(void);
#endif
