#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SR04.h"

static GPIO_InitTypeDef  GPIO_InitStructure;
static GPIO_InitTypeDef  GPIO_InitStruct;
TIM_OCInitTypeDef  		 TIM_OCInitStructure;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

void TIM13_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
	
		/*��ʱ���Ļ������ã��������ö�ʱ������������Ƶ��Ϊ100Hz */
	TIM_TimeBaseStructure.TIM_Period = (40000/100)-1;					//���ö�ʱ�����Ƶ��
	TIM_TimeBaseStructure.TIM_Prescaler = 2100-1;						//��һ�η�Ƶ�����ΪԤ��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;				//�ڶ��η�Ƶ,��ǰʵ��1��Ƶ��Ҳ���ǲ���Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);

	/* ����PF8 ����Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;					//��8������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				//���ø���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				//����ģʽ��������������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;			//����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;			//����Ҫ��������
	GPIO_Init(GPIOF, &GPIO_InitStructure);	
	
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource8, GPIO_AF_TIM13);
	
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//�����ͨ����PW1ģʽ�¹���
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//�����������
	TIM_OCInitStructure.TIM_Pulse = 200;						 //�Ƚ�ֵ����ռ�ձ�50%	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	 //�������Ч״̬Ϊ�ߵ�ƽ	
	TIM_OC1Init(TIM13, &TIM_OCInitStructure);
	
	TIM_Cmd(TIM13, ENABLE);
	Tim13_Set_Pwm(15,0);
}

void Tim13_Set_Pwm(uint32_t freq,uint32_t duty)
{
	uint32_t cmp=0;
	/* �ر�TIM13 */
	//TIM_Cmd(TIM13, DISABLE);
	
    /*��ʱ���Ļ������ã��������ö�ʱ������������Ƶ��Ϊ freq Hz */
    TIM_TimeBaseStructure.TIM_Period = (400000/freq)-1; //���ö�ʱ�����Ƶ��
    TIM_TimeBaseStructure.TIM_Prescaler = 2100-1; //��һ�η�Ƶ�����ΪԤ��Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);
	
    cmp = (TIM_TimeBaseStructure.TIM_Period+1) * duty/100;
    TIM_SetCompare1(TIM13,duty);	
	
	/* ʹ��TIM13 */
	//TIM_Cmd(TIM13, ENABLE);	
}

void Sr04_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;//����
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;//�����������������裬ĿǰΪ��
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	PBout(6)=0;
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	GPIO_Init(GPIOE,&GPIO_InitStruct);	
	
	
}

uint32_t Get_Sr04_Distance(void)
{
	uint32_t t=0;
	PBout(6)=1;
	Delay_us(10);
	PBout(6)=0;
	while(PEin(6)==0)
	{
		t++;
		Delay_us(1);
		
		//�����ʱ���ͷ���һ��������
		if(t>=1000000)
			return 0xFFFFFFFF;
	}
	
	t=0;
	while(PEin(6))
	{
		t++;
		Delay_us(9);//��������9us���ʹ�����3mm
	}
	
	return 3*t/2;
}

