#include "allhead.h"

static RTC_InitTypeDef  RTC_InitStructure;
static RTC_TimeTypeDef  RTC_TimeStructure;
static RTC_DateTypeDef  RTC_DateStructure;
static EXTI_InitTypeDef EXTI_InitStructure;
static NVIC_InitTypeDef NVIC_InitStructure;

extern EventGroupHandle_t g_event_group_rtc;

void rtc_init(void)
{
	 /* Enable the PWR clock ,ʹ�ܵ�Դʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to RTC ���������RTC*/
	PWR_BackupAccessCmd(ENABLE);
	

	/* ʹ��LSI*/
	RCC_LSICmd(ENABLE);
	
	/* ����LSI�Ƿ���Ч*/  
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	/* ѡ��LSI��ΪRTC��Ӳ��ʱ��Դ*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);


	/* ck_spre(1Hz) = RTCCLK /(uwAsynchPrediv + 1)/(uwSynchPrediv + 1)*/
	/* Enable the RTC Clock ��ʹ��RTCʱ��*/
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC APB registers synchronisation ���ȴ�RTC��ؼĴ�������*/
	RTC_WaitForSynchro();
 

	/* Configure the RTC data register and RTC prescaler������RTC���ݼĴ�����RTC�ķ�Ƶֵ */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;				//�첽��Ƶϵ��
	RTC_InitStructure.RTC_SynchPrediv = 0xF9;				//ͬ����Ƶϵ��
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;	//24Сʱ��ʽ
	RTC_Init(&RTC_InitStructure);

	if(RTC_ReadBackupRegister(RTC_BKP_DR0)==0x1234)
	{
		/* Set the date: Monday 2023/10/16 */
		RTC_DateStructure.RTC_Year = 0x23;
		RTC_DateStructure.RTC_Month = RTC_Month_October;
		RTC_DateStructure.RTC_Date = 0x16;
		RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Monday;
		RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

		/* Set the time to 14h 56mn 00s PM  */
		RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
		RTC_TimeStructure.RTC_Hours   = 0x20;
		RTC_TimeStructure.RTC_Minutes = 0x09;
		RTC_TimeStructure.RTC_Seconds = 0x30; 
		RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 	
	
	}

	
	//�رջ��ѹ���
	RTC_WakeUpCmd(DISABLE);
	
	//Ϊ���ѹ���ѡ��RTC���úõ�ʱ��Դ
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	
	//���û��Ѽ���ֵΪ�Զ����أ�д��ֵĬ����0
	RTC_SetWakeUpCounter(1-1);
	
	//���RTC�����жϱ�־
	RTC_ClearITPendingBit(RTC_IT_WUT);
	
	//ʹ��RTC�����ж�
	RTC_ITConfig(RTC_IT_WUT, ENABLE);

	//ʹ�ܻ��ѹ���
	RTC_WakeUpCmd(ENABLE);

	/* Configure EXTI Line22�������ⲿ�жϿ�����22 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;			//��ǰʹ���ⲿ�жϿ�����22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		//�ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//�����ش����ж� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;			//ʹ���ⲿ�жϿ�����22
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;		//����RTC�����жϴ���
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;	//��ռ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;		//��Ӧ���ȼ�Ϊ0x3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//ʹ��
	NVIC_Init(&NVIC_InitStructure);
	
	//�򱸷ݼĴ���0д��0x1234�������û���ǰRTCʱ���������Ѿ����ù�������Ҫ�ظ�����
	RTC_WriteBackupRegister(RTC_BKP_DR0,0x1234);
}

void RTC_WKUP_IRQHandler(void)
{
	BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	
	//����Ƿ��л����жϳ���
	if(RTC_GetITStatus(RTC_IT_WUT)==SET)
	{
		xEventGroupSetBitsFromISR(g_event_group_rtc,0x01,&xHigherPriorityTaskWoken);
		//����û�����
     	//��ձ�־λ
		RTC_ClearITPendingBit(RTC_IT_WUT);
		EXTI_ClearITPendingBit(EXTI_Line22);
	}
	if(xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}	
}
