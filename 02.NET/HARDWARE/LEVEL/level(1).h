#ifndef __LEVEL_H
#define __LEVEL_H

#include "stm32f10x.h"

//  引脚定义
#define LEVEL1_GPIO_CLK RCC_APB2Periph_GPIOA
#define LEVEL1_GPIO_PORT GPIOA
#define LEVEL1_GPIO_PIN GPIO_Pin_4

#define LEVEL2_GPIO_CLK RCC_APB2Periph_GPIOB
#define LEVEL2_GPIO_PORT GPIOB
#define LEVEL2_GPIO_PIN GPIO_Pin_4

#define LEVEL3_GPIO_CLK RCC_APB2Periph_GPIOB
#define LEVEL3_GPIO_PORT GPIOB
#define LEVEL3_GPIO_PIN GPIO_Pin_14

#define LEVEL1 GPIO_ReadInputDataBit(LEVEL1_GPIO_PORT, LEVEL1_GPIO_PIN) // 读取按键0
#define LEVEL2 GPIO_ReadInputDataBit(LEVEL2_GPIO_PORT, LEVEL2_GPIO_PIN) // 读取按键1
#define LEVEL3 GPIO_ReadInputDataBit(LEVEL3_GPIO_PORT, LEVEL3_GPIO_PIN) // 读取按键2

void Level_GPIO_Config(void);

#endif /* __LEVEL_H */
