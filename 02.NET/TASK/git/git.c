#include "git.h"

Data_TypeDef Data_init;						  // 设备数据结构体
Threshold_Value_TypeDef threshold_value_init; // 设备阈值设置结构体
Device_Satte_Typedef device_state_init;		  // 设备状态
DHT11_Data_TypeDef DHT11_Data;

// 获取数据参数
mySta Read_Data(Data_TypeDef *Device_Data)
{
	Read_DHT11(&DHT11_Data); // 获取温湿度数据
	Device_Data->temperatuer = DHT11_Data.temp_int + DHT11_Data.temp_deci * 0.01;
	Device_Data->humiditr = DHT11_Data.humi_int + DHT11_Data.humi_deci * 0.01;

	Device_Data->somg = Smog_Trans_Concentration();
	return MY_SUCCESSFUL;
}
// 初始化
mySta Reset_Threshole_Value(Threshold_Value_TypeDef *Value, Device_Satte_Typedef *device_state)
{


	// 状态重置

	Value->somg_value = 200;
	Value->humi_value = 70;
	Value->temp_value = 28;

	return MY_SUCCESSFUL;
}
// 更新OLED显示屏中内容
mySta Update_oled_massage()
{
#if OLED // 是否打开
	char str[50];
	// 发现明火
	if (LEVEL1 == 0)
	{
		sprintf(str, "监测到明火: 是 ");
		Data_init.fire = 1;
	}
	else
	{
		sprintf(str, "监测到明火: 否 ");
		Data_init.fire = 0;
	}
	OLED_ShowCH(0, 0, (unsigned char *)str);
	sprintf(str, "烟雾浓度: %d   ", Data_init.somg);
	OLED_ShowCH(0, 2, (unsigned char *)str);
	sprintf(str, "温度: %.1f      ", Data_init.temperatuer);
	OLED_ShowCH(0, 4, (unsigned char *)str);
	sprintf(str, "湿度: %.1f      ", Data_init.humiditr);
	OLED_ShowCH(0, 6, (unsigned char *)str);

#endif

	return MY_SUCCESSFUL;
}

// 更新设备状态
mySta Update_device_massage()
{
	// 自动模式
	if (Data_init.Flage == 0)
	{
		// 监测
		if (Data_init.temperatuer > threshold_value_init.temp_value || Data_init.humiditr > threshold_value_init.humi_value || Data_init.somg > threshold_value_init.somg_value || LEVEL1 == 0)
		{
			BEEP = ~BEEP;
		}
		else
		{
			BEEP = 0;
		}
	}
	// 回传数据
	if (Data_init.App)
	{
		switch (Data_init.App)
		{
		case 1:
			OneNet_SendMqtt(1); // 发送数据到APP
			break;
		case 2:
			OneNet_SendMqtt(2); // 发送数据到APP
			break;
		}
		Data_init.App = 0;
	}

	return MY_SUCCESSFUL;
}

// 定时器
void Automation_Close(void)
{
	// 实现30s自动切换界面
}
// 检测按键是否按下
static U8 num_on = 0;
static U8 key_old = 0;
void Check_Key_ON_OFF()
{
	U8 key;
	key = KEY_Scan(1);
	// 与上一次的键值比较 如果不相等，表明有键值的变化，开始计时
	if (key != 0 && num_on == 0)
	{
		key_old = key;
		num_on = 1;
	}
	if (key != 0 && num_on >= 1 && num_on <= Key_Scan_Time) // 25*10ms
	{
		num_on++; // 时间记录器
	}
	if (key == 0 && num_on > 0 && num_on < Key_Scan_Time) // 短按
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Short\n");
			if(Data_init.Flage == 0 ){
				Data_init.Flage = 1;
				BEEP = 1;
			}else{
				Data_init.Flage = 0;
				BEEP = 0;
			}
			break;
		case KEY2_PRES:
			printf("Key2_Short\n");

			break;
		case KEY3_PRES:
			printf("Key3_Short\n");

			break;

		default:
			break;
		}
		num_on = 0;
	}
	else if (key == 0 && num_on >= Key_Scan_Time) // 长按
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Long\n");

			break;
		case KEY2_PRES:
			printf("Key2_Long\n");

			break;
		case KEY3_PRES:
			printf("Key3_Long\n");

			break;
		default:
			break;
		}
		num_on = 0;
	}
}
// 解析json数据
mySta massage_parse_json(char *message)
{

	cJSON *cjson_test = NULL; // 检测json格式
	cJSON *cjson_data = NULL; // 数据
	const char *massage;
	// 定义数据类型
	u8 cjson_cmd; // 指令,方向

	/* 解析整段JSO数据 */
	cjson_test = cJSON_Parse(message);
	if (cjson_test == NULL)
	{
		// 解析失败
		printf("parse fail.\n");
		return MY_FAIL;
	}

	/* 依次根据名称提取JSON数据（键值对） */
	cjson_cmd = cJSON_GetObjectItem(cjson_test, "cmd")->valueint;
	/* 解析嵌套json数据 */
	cjson_data = cJSON_GetObjectItem(cjson_test, "data");

	switch (cjson_cmd)
	{
	case 0x01: // 消息包
		Data_init.Flage = cJSON_GetObjectItem(cjson_data, "flage")->valueint;
		Data_init.App = 1;
		break;
	case 0x02: // 消息包
		threshold_value_init.temp_value = cJSON_GetObjectItem(cjson_data, "temp_v")->valueint;
		threshold_value_init.humi_value = cJSON_GetObjectItem(cjson_data, "humi_v")->valueint;
		threshold_value_init.somg_value = cJSON_GetObjectItem(cjson_data, "somg_v")->valueint;
		Data_init.App = 1;
		break;
	case 0x03: // 数据包
		BEEP = cJSON_GetObjectItem(cjson_data, "beep")->valueint;
		Data_init.App = 1;
		break;
	case 0x04: // 数据包

		break;
	default:
		break;
	}

	/* 清空JSON对象(整条链表)的所有数据 */
	cJSON_Delete(cjson_test);

	return MY_SUCCESSFUL;
}
