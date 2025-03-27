#include "sys.h"
#include "key.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

void delay(void)
{
	uint32_t i=0xF000000;
	while(i--);
}

extern EventGroupHandle_t g_event_group_fpm;

void key_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;	
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	//ʹ�ܶ˿�A��Ӳ��ʱ�ӣ��˿�A���ܹ�����˵���˾��ǶԶ˿�A�ϵ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	//ʹ�ܶ˿�E��Ӳ��ʱ�ӣ��˿�E���ܹ�����˵���˾��ǶԶ˿�E�ϵ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		
	
	
	//ʹ��ϵͳ����Ӳ��ʱ�ӣ�˵���˾��Ƕ�ϵͳ���õ�Ӳ������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//����Ӳ��������GPIO���˿�A����0������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;							//��0 ������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;						//����ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;					//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;					//�����ⲿ�����������裬�ڲ�����Ҫ����������
	GPIO_Init(GPIOA,&GPIO_InitStructure);		

	//����Ӳ��������GPIO���˿�E����2 3 4������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	//��2 3 4������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;						//����ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;					//���Ÿ��ٹ���
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;					//�����ⲿ�����������裬�ڲ�����Ҫ���������衣�������Ҫʹ���ڲ���������
	GPIO_Init(GPIOE,&GPIO_InitStructure);			
	
	
	//��PA0�������ӵ�EXTI0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	
	//��PE2�������ӵ�EXTI2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);
	
	//��PE3�������ӵ�EXTI3
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);	

	//��PE4�������ӵ�EXTI4
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;	//EXTI0/2/3/4
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;							//�жϴ���
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 					//�½��أ��������£������������ж�����֪ͨCPU�������� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;									//ʹ��
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;							//�ⲿ�ж�0���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+3;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;							//�ⲿ�ж�2���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);		
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;							//�ⲿ�ж�3���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);		
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;							//�ⲿ�ж�3���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);
}


void EXTI0_IRQHandler(void)
{
	static BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	
	//����־λ
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		//���жϷ��������������¼���־���bit1��1���ȴ��¼���־������������һ��ʱ�ӽ��ĲŽ��������л�
		xEventGroupSetBitsFromISR(g_event_group_fpm,0x01,NULL);
		EXTI_ClearITPendingBit(EXTI_Line0);
	}	
	
	if(xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

void EXTI2_IRQHandler(void)
{
	static BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	

	//����־λ
	if(EXTI_GetITStatus(EXTI_Line2) == SET)
	{
		xEventGroupSetBitsFromISR(g_event_group_fpm,0x02,NULL);
		EXTI_ClearITPendingBit(EXTI_Line2);
	}

	/* �������¼���־����������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ��;
	   ����ȴ���һ��ʱ�ӽ��ĲŽ��������л�
	*/
	if(xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

void EXTI3_IRQHandler(void)
{

	static BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	
	//����־λ
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
	{
		xEventGroupSetBitsFromISR(g_event_group_fpm,0x04,NULL);
		EXTI_ClearITPendingBit(EXTI_Line3);
	}

	/* ��������Ϣ������������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ��;
	   ����ȴ���һ��ʱ�ӽ��ĲŽ��������л�
	*/
	if(xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}


void EXTI4_IRQHandler(void)
{
	static BaseType_t  xHigherPriorityTaskWoken = pdFALSE;	
	//����־λ
	if(EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		xEventGroupSetBitsFromISR(g_event_group_fpm,0x08,NULL);
		EXTI_ClearITPendingBit(EXTI_Line4);
	}

	/* ��������Ϣ������������ȼ����ڵ�ǰ���е��������˳��жϺ��������������л���ִ��ǰ��;
	   ����ȴ���һ��ʱ�ӽ��ĲŽ��������л�
	*/
	if(xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}	
}

