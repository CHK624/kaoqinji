#include "allhead.h"

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  

static USART_InitTypeDef   		USART_InitStructure;
static GPIO_InitTypeDef 		GPIO_InitStructure;
static NVIC_InitTypeDef   		NVIC_InitStructure;
static GPIO_InitTypeDef   		 GPIO_InitStruct;

static uint8_t  array[64]={0};
static uint32_t array_num=0;

volatile uint8_t  g_usart2_rx_buf[512];
volatile uint32_t g_usart2_rx_cnt=0;
volatile uint32_t g_usart2_rx_end=0;

volatile uint8_t  g_usart3_rx_buf[512];
volatile uint32_t g_usart3_rx_cnt=0;
volatile uint32_t g_usart3_rx_end=0;

extern EventGroupHandle_t g_event_group_rtc;
extern QueueHandle_t g_queue_rtc;
extern QueueHandle_t g_queue_flash;

#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;   

//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}


//��ʼ��IO ����1 
//bound:������
void Usart1_Init(u32 baud)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = baud;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	
	//USART_ClearFlag(USART1, USART_FLAG_TC);
	

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5;//��ռ���ȼ�5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =5;		//�����ȼ�5
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����

	USART_Cmd(USART1,ENABLE);
}

void Usart2_Init(uint32_t baud)
{
	//ʹ�ܶ˿�AӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	//ʹ�ܴ���AӲ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	//����PA2��PA3Ϊ���ù�������
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	//��PA2��PA3���ӵ�USART2��Ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	
	
	//����USART1����ز����������ʡ�����λ��У��λ
	USART_InitStructure.USART_BaudRate = baud;//������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//1λֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�����ڷ��ͺͽ�������
	USART_Init(USART2, &USART_InitStructure);
	
	
	//ʹ�ܴ��ڽ��յ����ݴ����ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//ʹ�ܴ���2����
	USART_Cmd(USART2,ENABLE);
}

void Usart3_Init(uint32_t baud)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;//����ģʽ
	GPIO_InitStruct.GPIO_Speed=GPIO_High_Speed;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;//����
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;//�����������������裬ĿǰΪ��
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

	
	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3, ENABLE);
}

void Usart_Send_Str(USART_TypeDef* USARTx,char *str)
{
	char *p = str;
	
	while(*p!='\0')
	{
		USART_SendData(USARTx,*p);
		
		p++;
	
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
		USART_ClearFlag(USARTx,USART_FLAG_TXE);
	}
}

void Usart_Send_Bytes(USART_TypeDef* USARTx,uint8_t *buf,uint32_t len)
{
	uint8_t *p = buf;
	
	while(len--)
	{
		USART_SendData(USARTx,*p);
		
		p++;
		
		//�ȴ����ݷ��ͳɹ�
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
		USART_ClearFlag(USARTx,USART_FLAG_TXE);
	}
}


//����1�жϷ������
void USART1_IRQHandler(void)                	
{
	uint8_t d;
	//BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  
	{
		//���մ�������
		d=USART_ReceiveData(USART1);

		array[array_num++] = d;
		
		if(d == '#' || array_num>=sizeof(array))
		{
			xQueueSendFromISR(g_queue_rtc,(void *)&array, NULL);
			memset(array,0,sizeof(array));
			array_num=0;
		}
		
		//��մ��ڽ����жϱ�־λ
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	} 
} 


#if 0
void USART2_IRQHandler(void)
{
	uint8_t d=0;
	
	//����Ƿ���յ�����
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		d=USART_ReceiveData(USART2);
		
		g_usart2_rx_buf[g_usart2_rx_cnt++]=d;
		
		if(g_usart2_rx_cnt >= sizeof g_usart2_rx_buf)
		{
			g_usart2_rx_end=1;
		}
#if 0		
		USART_SendData(USART1,d);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
#endif		
		//��ձ�־λ��������Ӧ�µ��ж�����
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}
#endif


void USART3_IRQHandler(void)
{
	uint8_t d=0;
	//BaseType_t  xHigherPriorityTaskWoken = pdFALSE;
	//����Ƿ���յ�����
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		//���մ�������
		d=USART_ReceiveData(USART3);

		array[array_num++] = d;
		
		if(d == '#' || array_num>=sizeof(array))
		{
			xQueueSendFromISR(g_queue_rtc,(void *)&array, NULL);
			memset(array,0,sizeof(array));
			array_num=0;
		}
		
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}



