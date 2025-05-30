#include "adc.h"
#include "delay.h"

__IO uint16_t ADC_ConvertedValue[NOFCHANEL] = {0};

/**
 * @brief  ADC GPIO 初始化
 * @param  无
 * @retval 无
 */
static void ADCx_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// 打开 ADC IO端口时钟
	ADC_GPIO_APBxClock_FUN(ADC_GPIO_CLK, ENABLE);

	// 配置 ADC IO 引脚模式
	GPIO_InitStructure.GPIO_Pin = ADC_PIN2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

	// 初始化 ADC IO
	GPIO_Init(ADC_PORT, &GPIO_InitStructure);
}

/**
 * @brief  配置ADC工作模式
 * @param  无
 * @retval 无
 */
static void ADCx_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	// 打开DMA时钟
	RCC_AHBPeriphClockCmd(ADC_DMA_CLK, ENABLE);
	// 打开ADC时钟
	ADC_APBxClock_FUN(ADC_CLK, ENABLE);

	// 复位DMA控制器
	DMA_DeInit(ADC_DMA_CHANNEL);

	// 配置 DMA 初始化结构体
	// 外设基址为：ADC 数据寄存器地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(ADCx->DR));

	// 存储器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)ADC_ConvertedValue;

	// 数据源来自外设
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

	// 缓冲区大小，应该等于数据目的地的大小
	DMA_InitStructure.DMA_BufferSize = NOFCHANEL;

	// 外设寄存器只有一个，地址不用递增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

	// 存储器地址递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;

	// 外设数据大小为半字，即两个字节
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;

	// 内存数据大小也为半字，跟外设数据大小相同
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

	// 循环传输模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

	// DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;

	// 禁止存储器到存储器模式，因为是从外设到存储器
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	// 初始化DMA
	DMA_Init(ADC_DMA_CHANNEL, &DMA_InitStructure);

	// 使能 DMA 通道
	DMA_Cmd(ADC_DMA_CHANNEL, ENABLE);

	// ADC 模式配置
	// 只使用一个ADC，属于单模式
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;

	// 扫描模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;

	// 连续转换模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;

	// 不用外部触发转换，软件开启即可
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

	// 转换结果右对齐
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;

	// 转换通道个数
	ADC_InitStructure.ADC_NbrOfChannel = NOFCHANEL;

	// 初始化ADC
	ADC_Init(ADCx, &ADC_InitStructure);

	// 配置ADC时钟Ｎ狿CLK2的8分频，即9MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);

	// 配置ADC 通道的转换顺序和采样时间
	ADC_RegularChannelConfig(ADCx, ADC_CHANNEL2, 1, ADC_SampleTime_55Cycles5);
	// ADC_RegularChannelConfig(ADCx, ADC_CHANNEL2, 2, ADC_SampleTime_55Cycles5);
	//	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL3, 3, ADC_SampleTime_55Cycles5);
	//	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL4, 4, ADC_SampleTime_55Cycles5);
	//	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL5, 5, ADC_SampleTime_55Cycles5);
	//	ADC_RegularChannelConfig(ADC_x, ADC_CHANNEL6, 6, ADC_SampleTime_55Cycles5);

	// 使能ADC DMA 请求
	ADC_DMACmd(ADCx, ENABLE);

	// 开启ADC ，并开始转换
	ADC_Cmd(ADCx, ENABLE);

	// 初始化ADC 校准寄存器
	ADC_ResetCalibration(ADCx);
	// 等待校准寄存器初始化完成
	while (ADC_GetResetCalibrationStatus(ADCx))
		;

	// ADC开始校准
	ADC_StartCalibration(ADCx);
	// 等待校准完成
	while (ADC_GetCalibrationStatus(ADCx))
		;

	// 由于没有采用外部触发，所以使用软件触发ADC转换
	ADC_SoftwareStartConvCmd(ADCx, ENABLE);
}

/**
 * @brief  ADC初始化
 * @param  无
 * @retval 无
 */
void ADCx_Init(void)
{
	ADCx_GPIO_Config();
	ADCx_Mode_Config();
}
// 读取烟雾传感器的电压值
u16 Light_value(void)
{
	u16 temp_val = 0;
	temp_val = 410 - ADC_ConvertedValue[0] / 10; // 读取ADC值

	return (u16)temp_val;
}
// 分析从烟雾传感器获取的电压值，通过公式计算出可燃气体的浓度值
// 设Rs为传感器的体电阻，其中气体浓度上升，将导致Rs下降。而Rs的下降则会导致，MQ-2的4脚、6脚对地输出的电压增大。
// 所以气体浓度增大，其输出的电压也会增大。因Rs在不同气体中有不同的浓度值，所以该函数仅作为参考.
u16 Smog_Trans_Concentration(void)
{

	u16 temp_val;
	temp_val = ADC_ConvertedValue[0] / 10; // 读取ADC值
	//	u16 Rs;
	//	Rs = SMOG_PIN46_R*(4096.0/temp_val - 1);
	//	//printf("Smog_Rs_Val:%d\r\n", Rs);

	return temp_val;
}

/*********************************************END OF FILE**********************/
