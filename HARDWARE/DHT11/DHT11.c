#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DHT11.h"


static GPIO_InitTypeDef GPIO_InitStruct;

void DHT_Init(uint32_t dht_flag)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	
	
	if(dht_flag == 0)
	{
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	}
	else
	{
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	}
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;//��©
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;//�����������������裬ĿǰΪ��
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
	PGout(9)=1;
}


int Get_DHT_Data(uint32_t *arr)
{
	uint32_t time=0;
	int32_t i=0;
	int32_t j=0;
	uint8_t d=0;
	uint8_t check_num=0;
	DHT_Init(0);
	PGout(9)=0;
	Delay_ms(18);
	PGout(9)=1;
	Delay_us(30);
	DHT_Init(1);//����PG9������Ϊ����ģʽ
	time=0;
	while(PGin(9))//�ȴ��͵�ƽ
	{
		time++;
		Delay_us(1);
		if(time >= 4000)
			return -1;
	}
	time=0;
	while(PGin(9)==0)//�͵�ƽ80us
	{
		time++;
		Delay_us(1);
		if(time>=90)
			return -2;
	}
	time=0;
	while(PGin(9)) //�ߵ�ƽ80us
	{
		time++;
		Delay_us(1);
		if(time>=90)
			return -2;
	}
	for(i=0;i<5;i++)
	{
		d=0;
		//��ȡһ���ֽ�
		for(j=7;j>=0;j--)
		{
			//��ȡһλ����
			time=0;
			while(PGin(9)==0)//�͵�ƽ80us
			{
				time++;
				Delay_us(1);
				if(time>=100)
					return -4;
			}
			Delay_us(40);//��ʱ��ΧҪ��28us~70us����ֹ̫�̼�⵽��������0�ĸߵ�ƽ��̫����⵽��������1�ĵ͵�ƽ
			if(PGin(9))//PGin(9)����40us���Ǹߵ�ƽ����Ϊ����1
			{
				d|=1<<j;
				time=0;
				while(PGin(9))//�ȴ��ߵ�ƽ����
				{
					time++;
					Delay_us(1);
					
					if(time >= 100)
						return -5;
				}
			}
		}
		arr[i]=d;
	}
	Delay_us(50);//���Ե͵�ƽ
	check_num=arr[0]+arr[1]+arr[2]+arr[3];
	if(check_num != arr[4])//����У��ͣ�����Ƿ����
	{
		return -6;
	}
	return 0;
}
