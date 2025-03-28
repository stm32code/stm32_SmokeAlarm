#include "git.h"

Data_TypeDef Data_init;						  // �豸���ݽṹ��
Threshold_Value_TypeDef threshold_value_init; // �豸��ֵ���ýṹ��
Device_Satte_Typedef device_state_init;		  // �豸״̬
DHT11_Data_TypeDef DHT11_Data;

// ��ȡ���ݲ���
mySta Read_Data(Data_TypeDef *Device_Data)
{
	Read_DHT11(&DHT11_Data); // ��ȡ��ʪ������
	Device_Data->temperatuer = DHT11_Data.temp_int + DHT11_Data.temp_deci * 0.01;
	Device_Data->humiditr = DHT11_Data.humi_int + DHT11_Data.humi_deci * 0.01;

	Device_Data->somg = Smog_Trans_Concentration();
	return MY_SUCCESSFUL;
}
// ��ʼ��
mySta Reset_Threshole_Value(Threshold_Value_TypeDef *Value, Device_Satte_Typedef *device_state)
{


	// ״̬����

	Value->somg_value = 200;
	Value->humi_value = 70;
	Value->temp_value = 28;

	return MY_SUCCESSFUL;
}
// ����OLED��ʾ��������
mySta Update_oled_massage()
{
#if OLED // �Ƿ��
	char str[50];
	// ��������
	if (LEVEL1 == 0)
	{
		sprintf(str, "��⵽����: �� ");
		Data_init.fire = 1;
	}
	else
	{
		sprintf(str, "��⵽����: �� ");
		Data_init.fire = 0;
	}
	OLED_ShowCH(0, 0, (unsigned char *)str);
	sprintf(str, "����Ũ��: %d   ", Data_init.somg);
	OLED_ShowCH(0, 2, (unsigned char *)str);
	sprintf(str, "�¶�: %.1f      ", Data_init.temperatuer);
	OLED_ShowCH(0, 4, (unsigned char *)str);
	sprintf(str, "ʪ��: %.1f      ", Data_init.humiditr);
	OLED_ShowCH(0, 6, (unsigned char *)str);

#endif

	return MY_SUCCESSFUL;
}

// �����豸״̬
mySta Update_device_massage()
{
	// �Զ�ģʽ
	if (Data_init.Flage == 0)
	{
		// ���
		if (Data_init.temperatuer > threshold_value_init.temp_value || Data_init.humiditr > threshold_value_init.humi_value || Data_init.somg > threshold_value_init.somg_value || LEVEL1 == 0)
		{
			BEEP = ~BEEP;
		}
		else
		{
			BEEP = 0;
		}
	}
	// �ش�����
	if (Data_init.App)
	{
		switch (Data_init.App)
		{
		case 1:
			OneNet_SendMqtt(1); // �������ݵ�APP
			break;
		case 2:
			OneNet_SendMqtt(2); // �������ݵ�APP
			break;
		}
		Data_init.App = 0;
	}

	return MY_SUCCESSFUL;
}

// ��ʱ��
void Automation_Close(void)
{
	// ʵ��30s�Զ��л�����
}
// ��ⰴ���Ƿ���
static U8 num_on = 0;
static U8 key_old = 0;
void Check_Key_ON_OFF()
{
	U8 key;
	key = KEY_Scan(1);
	// ����һ�εļ�ֵ�Ƚ� �������ȣ������м�ֵ�ı仯����ʼ��ʱ
	if (key != 0 && num_on == 0)
	{
		key_old = key;
		num_on = 1;
	}
	if (key != 0 && num_on >= 1 && num_on <= Key_Scan_Time) // 25*10ms
	{
		num_on++; // ʱ���¼��
	}
	if (key == 0 && num_on > 0 && num_on < Key_Scan_Time) // �̰�
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
	else if (key == 0 && num_on >= Key_Scan_Time) // ����
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
// ����json����
mySta massage_parse_json(char *message)
{

	cJSON *cjson_test = NULL; // ���json��ʽ
	cJSON *cjson_data = NULL; // ����
	const char *massage;
	// ������������
	u8 cjson_cmd; // ָ��,����

	/* ��������JSO���� */
	cjson_test = cJSON_Parse(message);
	if (cjson_test == NULL)
	{
		// ����ʧ��
		printf("parse fail.\n");
		return MY_FAIL;
	}

	/* ���θ���������ȡJSON���ݣ���ֵ�ԣ� */
	cjson_cmd = cJSON_GetObjectItem(cjson_test, "cmd")->valueint;
	/* ����Ƕ��json���� */
	cjson_data = cJSON_GetObjectItem(cjson_test, "data");

	switch (cjson_cmd)
	{
	case 0x01: // ��Ϣ��
		Data_init.Flage = cJSON_GetObjectItem(cjson_data, "flage")->valueint;
		Data_init.App = 1;
		break;
	case 0x02: // ��Ϣ��
		threshold_value_init.temp_value = cJSON_GetObjectItem(cjson_data, "temp_v")->valueint;
		threshold_value_init.humi_value = cJSON_GetObjectItem(cjson_data, "humi_v")->valueint;
		threshold_value_init.somg_value = cJSON_GetObjectItem(cjson_data, "somg_v")->valueint;
		Data_init.App = 1;
		break;
	case 0x03: // ���ݰ�
		BEEP = cJSON_GetObjectItem(cjson_data, "beep")->valueint;
		Data_init.App = 1;
		break;
	case 0x04: // ���ݰ�

		break;
	default:
		break;
	}

	/* ���JSON����(��������)���������� */
	cJSON_Delete(cjson_test);

	return MY_SUCCESSFUL;
}
