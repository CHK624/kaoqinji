#include "led.h" 

void LED_Init(void)
{    	 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);	//使能GPIOF时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	//GPIOF9,F10初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;	//9号和10号引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉
	GPIO_Init(GPIOF, &GPIO_InitStructure);					//初始化GPIO
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;	//9号和10号引脚
	GPIO_Init(GPIOE, &GPIO_InitStructure);					//初始化GPIO
	
	
	GPIO_SetBits(GPIOF,GPIO_Pin_9 | GPIO_Pin_10);			//GPIOF9,F10设置高，灯灭
	GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);			//GPIOF9,F10设置高，灯灭
	//GPIO_ResetBits
}






