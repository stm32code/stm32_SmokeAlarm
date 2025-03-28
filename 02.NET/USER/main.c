#include "git.h"

// �����ʱ���趨
static Timer task1_id;
static Timer task2_id;
static Timer task3_id;

// ��ȡȫ�ֱ���
const char *topics[] = {S_TOPIC_NAME};
extern u32 time_4; // ms ��ʱ����
// Ӳ����ʼ��
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    HZ = GB16_NUM();                                // ����
    delay_init();                                   // ��ʱ������ʼ��
    GENERAL_TIM_Init(TIM_4, 0, 1);
    Usart1_Init(115200); // ����1��ʼ��Ϊ115200
    USART2_Init(115200); // ����2������ESP8266��

  
#if OLED // OLED�ļ�����
    OLED_Init();
    OLED_ColorTurn(0);   // 0������ʾ��1 ��ɫ��ʾ
    OLED_DisplayTurn(0); // 0������ʾ 1 ��Ļ��ת��ʾ
#endif

    while (Reset_Threshole_Value(&threshold_value_init, &device_state_init) != MY_SUCCESSFUL)
        delay_ms(5); // ��ʼ����ֵ

#if OLED // OLED�ļ�����
    OLED_Clear();
#endif
}
// �����ʼ��
void Net_Init()
{
#if OLED // OLED�ļ�����
    char str[50];
    OLED_Clear();
    // дOLED����
    sprintf(str, "-���WIFI�ȵ�");
    OLED_ShowCH(0, 0, (unsigned char *)str);
    sprintf(str, "-����:%s         ", SSID);
    OLED_ShowCH(0, 2, (unsigned char *)str);
 
#endif
    ESP8266_Init();          // ��ʼ��ESP8266
    while (OneNet_DevLink()) // ����OneNET
        delay_ms(300);
    while (OneNet_Subscribe(topics, 1)) // ��������
        delay_ms(300);

    Connect_Net = 60; // �����ɹ�
#if OLED              // OLED�ļ�����
    OLED_Clear();
#endif
}

// ����1
void task1(void)
{
		OneNet_SendData();
}
// ����2
void task2(void)
{
    char str[50];
// �豸����
#if NETWORK_CHAEK
    if (Connect_Net == 180) {

#if OLED // OLED�ļ�����
        OLED_Clear();
        // дOLED����
        sprintf(str, "-���WIFI�ȵ�");
        OLED_ShowCH(0, 0, (unsigned char *)str);
        sprintf(str, "-����:%s         ", SSID);
   
        OLED_ShowCH(0, 6, (unsigned char *)str);
#endif
        ESP8266_Init();          // ��ʼ��ESP8266
        while (OneNet_DevLink()) // ����OneNET
            delay_ms(300);
        while (OneNet_Subscribe(topics, 1)) // ��������
            delay_ms(300);
        Connect_Net = 60; // �����ɹ�
#if OLED                  // OLED�ļ�����
        OLED_Clear();
#endif
    }
#endif
 
                             // BEEP  = ~BEEP;
    State = ~State;
}
// ����3
void task3(void)
{
    // 10��һ��
    if (Connect_Net && Data_init.App == 0) {
        Data_init.App = 1;
    }
}
// �����ʼ��
void SoftWare_Init(void)
{
    // ��ʱ����ʼ��
    timer_init(&task1_id, task1, 60000, 1); // 30S
    timer_init(&task2_id, task2, 80, 1);    // 80ms
    timer_init(&task3_id, task3, 3000, 1);  // 3S
 
}
// ������
int main(void)
{

    unsigned char *dataPtr = NULL;
    SoftWare_Init(); // �����ʼ��
    Hardware_Init(); // Ӳ����ʼ��
    // ������ʾ
    BEEP = 1;
    delay_ms(100);
    BEEP = 0;
    Net_Init();            // �����ʼ
    TIM_Cmd(TIM4, ENABLE); // ʹ�ܼ�����

    while (1) {

        // �߳�
        timer_loop(); // ��ʱ��ִ��
        // ���ڽ����ж�
        dataPtr = ESP8266_GetIPD(0);
        if (dataPtr != NULL) {
            OneNet_RevPro(dataPtr); // ��������
        }
				#if KEY_OPEN
				if (time_4 % 25 == 0)
				{
					// ��������
					Check_Key_ON_OFF();
				}
				#endif
    }
}
