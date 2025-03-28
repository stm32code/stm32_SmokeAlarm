#include "flash.h"
#include "usart.h"



/*********************************************************************************
 * @Function	:  内部FLASH写入时，等待空闲
 * @Input		:  timeOut,等待BSY标志空闲
 * @Output		:  None
 * @Return		:  0：闲
 *         		 1，忙
 * @Others		:  None
 * @Date			:  2022-08-23
 **********************************************************************************/
// 作用：内部FLASH写入时，等待空闲，BSY位标志:1忙
static uint8_t waitForFlashBSY(uint32_t timeOut)
{
    while ((FLASH->SR & 0x01) && (timeOut-- != 0x00))
        ; // 等待BSY标志空闲
    if (timeOut == 0)
        return 1; // 失败，返回1, 等待超时；　

    return 0; // 正常，返回０
}
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
uint8_t System_WriteInteriorFlash(uint32_t writeAddr, uint8_t *writeToBuffer, uint16_t numToWrite)
{
    uint16_t flashSize = *(uint16_t *)(0x1FFFF7E0); // 读取芯片FLASH大小;本寄存器值为芯片出厂前写入的FLASH大小，只读，单位：KByte

    uint32_t addrOff = writeAddr - STM32_FLASH_ADDR_BASE; // 去掉0x08000000后的实际偏移地址
    uint32_t secPos = addrOff / STM32_FLASH_SECTOR_SIZE;
    ;                                                      // 扇区地址,即起始地址在第几个扇区
    uint16_t secOff = addrOff % STM32_FLASH_SECTOR_SIZE;   // 开始地始偏移字节数: 数据在扇区的第几字节存放
    uint16_t secRemain = STM32_FLASH_SECTOR_SIZE - secOff; // 本扇区需要写入的字节数 ,用于判断够不够存放余下的数据

    // 判断地址有效性
    if (writeAddr < STM32_FLASH_ADDR_BASE)
        return 1; // 如果读的地址，小于FLASH的最小地址，则退出，返回1_地址失败
    if (writeAddr > (STM32_FLASH_ADDR_BASE + (flashSize * 1024)))
        return 1; // 如果读的地址，超出FLASH的最大地址，则退出, 返回1_地址失败

    // 0_解锁FLASH
    FLASH->KEYR = ((uint32_t)0x45670123);
    FLASH->KEYR = ((uint32_t)0xCDEF89AB);

    if (numToWrite <= secRemain)
        secRemain = numToWrite;
    while (1)
    {
        // 1_读取当前页的数据
        if (waitForFlashBSY(0x00888888))
            return 2;                                                                                                                  // 失败，返回:2, 失败原因：FLASH->SR:BSY忙超时
        System_ReadInteriorFlash(secPos * STM32_FLASH_SECTOR_SIZE + STM32_FLASH_ADDR_BASE, sectorbufferTemp, STM32_FLASH_SECTOR_SIZE); // 读取扇区内容到缓存

        // 2_擦险指定页(扇区)
        if (waitForFlashBSY(0x00888888))
            return 2;                                                         // 失败，返回:2, 失败原因：FLASH->SR:BSY忙超时
        FLASH->CR |= 1 << 1;                                                  // PER:选择页擦除；位2MER为全擦除
        FLASH->AR = STM32_FLASH_ADDR_BASE + secPos * STM32_FLASH_SECTOR_SIZE; // 填写要擦除的页地址
        FLASH->CR |= 0x40;                                                    // STRT:写1时触发一次擦除运作　
        if (waitForFlashBSY(0x00888888))
            return 2;                        // 失败，返回:３, 失败原因：擦除超时
        FLASH->CR &= ((uint32_t)0x00001FFD); // 关闭页擦除功能

        for (uint16_t i = 0; i < secRemain; i++) // 原始数据写入缓存
            sectorbufferTemp[secOff + i] = writeToBuffer[i];

        for (uint16_t i = 0; i < STM32_FLASH_SECTOR_SIZE / 2; i++)
        { // 缓存数据写入芯片FLASH
            if (waitForFlashBSY(0x00888888))
                return 2;                                                                                                                                   // 失败，返回:2, 失败原因：FLASH->SR:BSY忙超时
            FLASH->CR |= 0x01 << 0;                                                                                                                         // PG: 编程
            *(uint16_t *)(STM32_FLASH_ADDR_BASE + secPos * STM32_FLASH_SECTOR_SIZE + i * 2) = (sectorbufferTemp[i * 2 + 1] << 8) | sectorbufferTemp[i * 2]; // 缓存数据写入设备

            if (waitForFlashBSY(0x00888888))
                return 2;                        // 失败，返回:2, 失败原因：FLASH->SR:BSY忙超时
            FLASH->CR &= ((uint32_t)0x00001FFE); // 关闭编程
        }

        if (secRemain == numToWrite)
        {
            break; // 已全部写入
        }
        else
        {                                                                                                // 未写完
            writeToBuffer += secRemain;                                                                  // 原始数据指针偏移
            secPos++;                                                                                    // 新扇区
            secOff = 0;                                                                                  // 新偏移位,扇区内数据起始地址
            numToWrite -= secRemain;                                                                     // 剩余未写字节数
            secRemain = (numToWrite > STM32_FLASH_SECTOR_SIZE) ? (STM32_FLASH_SECTOR_SIZE) : numToWrite; // 计算新扇区写入字节数
        }
    }
    FLASH->CR |= 1 << 7; // LOCK:重新上锁

    return 0;
}
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
uint8_t System_ReadInteriorFlash(uint32_t readAddr, uint8_t *pBuffer, uint16_t numToRead)
{
    // 获取芯片FLASH大小
    uint16_t flashSize = *(uint16_t *)(0x1FFFF7E0); // 读取芯片FLASH大小;本寄存器值为芯片出厂前写入的FLASH大小，只读单位：KByte

    // 判断地址有效性
    if (readAddr < STM32_FLASH_ADDR_BASE)
        return 1; // 如果读的地址，小于FLASH的最小地址，则退出
    if (readAddr > (STM32_FLASH_ADDR_BASE + (flashSize * 1024)))
        return 2; // 如果读的地址，超出FLASH的最大地址，则退出

    // 开始复制
    while (numToRead--)
    {
        *pBuffer = *(__IO uint8_t *)readAddr;
        pBuffer++;  // 指针后移一个数据长度
        readAddr++; // 偏移一个数据位
    }

    return 0; // 成功，返回0;
}
// 写入数据
U32 addrShort1 = 0x0800C000 + 10 * 16; // 1
U32 addrShort2 = 0x0800C000 + 12 * 16; // 2

void W_Test(void)
{

    System_WriteInteriorFlash(addrShort1, (uint8_t *)&threshold_value_init.Distance_value, sizeof(threshold_value_init.Distance_value)); // 存入数据，函数内自动解锁，擦除，写入
    // System_ReadInteriorFlash(addrShort1, (uint8_t *)&device_state_init.Drug1, sizeof(device_state_init.Drug1));  // 读取数据，函数内自动按半字读取
    // printf("1地址:0x%X 写入:%d  读取:%d \r\n", addrShort1, device_state_init.Drug1, device_state_init.Drug1);

    //System_WriteInteriorFlash(addrShort2, (uint8_t *)&latitude_sum, sizeof(latitude_sum)); // 存入数据，函数内自动解锁，擦除，写入
    // System_ReadInteriorFlash(addrShort2, (uint8_t *)&device_state_init.Drug2, sizeof(device_state_init.Drug2));  // 读取数据，函数内自动按半字读取
    // printf("2地址:0x%X 写入:%d  读取:%d \r\n", addrShort3, device_state_init.Drug2, device_state_init.Drug2);
}
// 读取数据
void R_Test(void)
{
    // System_WriteInteriorFlash(addrShort1, (uint8_t *)&device_state_init.Drug1, sizeof(device_state_init.Drug1)); // 存入数据，函数内自动解锁，擦除，写入
    System_ReadInteriorFlash(addrShort1, (uint8_t *)&threshold_value_init.Distance_value, sizeof(threshold_value_init.Distance_value)); // 读取数据，函数内自动按半字读取
    //printf("1地址:0x%X 读取:%.6f \r\n", addrShort1, longitude_sum);

    //  System_WriteInteriorFlash(addrShort2, (uint8_t *)&device_state_init.Drug2, sizeof(device_state_init.Drug2)); // 存入数据，函数内自动解锁，擦除，写入
    //System_ReadInteriorFlash(addrShort2, (uint8_t *)&latitude_sum, sizeof(latitude_sum)); // 读取数据，函数内自动按半字读取
    //printf("2地址:0x%X 读取:%.6f \r\n", addrShort2, latitude_sum);
}
/*********************************************************************************
 * @Function	:  测试FALSH读写程序
 * @Input		:  None
 * @Output		:  None
 * @Return		:  None
 * @Others		:  演示程序的几种数据读写方法
 * @Date			:  2022-08-23
 **********************************************************************************/
void W_R_Test(void)
{

    // // 测试1：字符
    // uint32_t addrChar = 0x08000000 + 38 * 1024; // 注意地址必须是偶数，且地址段尽量安排在FLASH尾部，避免覆盖了代码段数据
    // char writeChar = 'F';
    // System_WriteInteriorFlash(addrChar, (uint8_t *)&writeChar, 1); // 存入数据，函数内自动解锁，擦除，写入
    // char readChar = 0;
    // System_ReadInteriorFlash(addrChar, (uint8_t *)&readChar, 1); // 读取数据，函数内自动按半字读取
    // printf("测试char     地址:0x%X    写入:%c       读取:%c \r\n", addrChar, writeChar, readChar);
    printf("\r\n>>>>开始测试内部FLASH读写，及数据转换：\r\n");
    // 测试2：带符号的16位
    uint32_t addrShort = 0x08000000 + 2 * 1024; // 注意地址必须是偶数，且地址段尽量安排在FLASH尾部，避免覆盖了代码段数据
    short writeShort = -888;
    System_WriteInteriorFlash(addrShort, (uint8_t *)&writeShort, sizeof(writeShort)); // 存入数据，函数内自动解锁，擦除，写入
    short readShort = 0;
    System_ReadInteriorFlash(addrShort, (uint8_t *)&readShort, sizeof(writeShort)); // 读取数据，函数内自动按半字读取
    printf("测试short    地址:0x%X    写入:%d    读取:%d \r\n", addrShort, writeShort, readShort);

    // // 测试3：无符号的32位
    // uint32_t addrUint = 0x08000000 + 254 * 1024; // 注意地址必须是偶数，且地址段尽量安排在FLASH尾部，避免覆盖了代码段数据
    // uint32_t writeUint = 0x12345678;
    // System_WriteInteriorFlash(addrUint, (uint8_t *)&writeUint, 4); // 存入数据，函数内自动解锁，擦除，写入
    // uint32_t readUint = 0;
    // System_ReadInteriorFlash(addrUint, (uint8_t *)&readUint, 4); // 读取数据，函数内自动按半字读取
    // printf("测试uint     地址:0x%X    写入:0x%X     读取:0x%X \r\n", addrUint, writeUint, readUint);

    // // 测试4：字符串
    // uint32_t addrString = 0x08000000 + 166 * 1024 - 6; // 注意地址必须是偶数，且地址段尽量安排在FLASH尾部，避免覆盖了代码段数据
    // char writeString[100] = "早吖! 去哪呢?";
    // System_WriteInteriorFlash(addrString, (uint8_t *)writeString, sizeof(writeString)); // 存入数据，函数内自动解锁，擦除，写入
    // char readString[100];
    // System_ReadInteriorFlash(addrString, (uint8_t *)readString, sizeof(readString)); // 读取数据，函数内自动按半字读取
    // printf("测试string   地址:0x%X　  写入:%s  读取:%s \r\n", addrString, writeString, readString);
}
