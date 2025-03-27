#include "allhead.h"

/*
ָ��ģ��:���Խ������ָ�ơ���ָ֤�ơ���ȡ���������ָ�ƣ�ͬʱOLED��ʾ������ָ����֤�ɹ�����������������LED����ʧ�ܳ�����LED����
OLED��LED�������������������
��Ĥ����:���°���������������OLED��ʾ��������
����KEY1���ڣ������������У�
��������󣬰���KEY2.3.4�ֱ�ʵ����� ��ȡ���� ���ָ�ƵĹ��ܣ�ͨ������1/�����޸�ʱ������ڣ�����OLEDʵʱ��ʾ����
���ӽ��ܷ�

*/

#define	XOR_KEY	0x17

volatile uint32_t	rec_cnt=0;
volatile uint32_t	i=0;
char 	 save_buf[128]="0";
char 	 att_buf[30]="0";

static RTC_TimeTypeDef  RTC_TimeStructure;
static RTC_DateTypeDef  RTC_DateStructure;

TaskHandle_t app_task_init_handle     = NULL;
TaskHandle_t app_task_oled_handle     = NULL;
TaskHandle_t app_task_keyboard_handle = NULL;
TaskHandle_t app_task_led_handle 	  = NULL;
TaskHandle_t app_task_rtc_handle 	  = NULL;
TaskHandle_t app_task_root_handle 	  = NULL;
TaskHandle_t app_task_beep_handle 	  = NULL;
TaskHandle_t app_task_atte_handle 	  = NULL;
TaskHandle_t app_task_mod_handle 	  = NULL;
TaskHandle_t app_task_show_handle 	  = NULL;

QueueHandle_t g_queue_oled;         //oled��Ϣ����
QueueHandle_t g_queue_rtc; 			//rtc��Ϣ����
QueueHandle_t g_queue_flash; 		//flash��Ϣ����

SemaphoreHandle_t g_mutex_oled;		//oled������
SemaphoreHandle_t g_mutex_printf;	//printf������

EventGroupHandle_t g_event_group_fpm; //ָ���¼���־��
EventGroupHandle_t g_event_group_led; //LED�¼���־��
EventGroupHandle_t g_event_group_root;//root�¼���־��
EventGroupHandle_t g_event_group_beep;//beep�¼���־��
EventGroupHandle_t g_event_group_rtc; //rtc�¼���־��


/*   ����menu 	*/ 
static void app_task_init(void* pvParameters);  
/*   ����oled 	*/ 
static void app_task_oled(void* pvParameters);  
/* ����keyboard */ 
static void app_task_keyboard(void* pvParameters);  
/*   ����led 	*/
static void app_task_led(void* pvParameters);
/*   ����root 	*/
static void app_task_root(void* pvParameters);
/*   ����atte 	*/
static void app_task_atte(void* pvParameters);
/*   ����beep 	*/
static void app_task_beep(void* pvParameters);
/*   ����rtc 	*/
static void app_task_rtc(void* pvParameters);  
/*   ����mod 	*/
static void app_task_mod(void* pvParameters);
/* ����show */ 
static void app_task_show(void* pvParameters); 

void exceptions_catch(void)
{
	uint32_t icsr=SCB->ICSR;
	uint32_t ipsr=__get_IPSR();

	dgb_printf_safe("exceptions catch\r\n"); 
	dgb_printf_safe("IPSR=0x%08X\r\n",ipsr); 
	dgb_printf_safe("ICSR=0x%08X\r\n",icsr);
 }

 /* OLED�������߶ȷ�װ */
#define OLED_SAFE(__CODE)                                \
	{                                                    \
                                                         \
		do                                               \
		{                                                \
			xSemaphoreTake(g_mutex_oled, portMAX_DELAY); \
			if (g_oled_display_flag)                     \
				__CODE;                                  \
			xSemaphoreGive(g_mutex_oled);                \
		} while (0)                                      \
	}

#define DEBUG_PRINTF_EN 1
 
 void dgb_printf_safe(const char *format, ...)
{
#if DEBUG_PRINTF_EN

	va_list args;
	va_start(args, format);

	/* ��ȡ�����ź��� */
	xSemaphoreTake(g_mutex_printf, portMAX_DELAY);

	vprintf(format, args);

	/* �ͷŻ����ź��� */
	xSemaphoreGive(g_mutex_printf);

	va_end(args);
#else
	(void)0;
#endif
}

int main(void)
{
	/* ����ϵͳ�ж����ȼ�����4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* ϵͳ��ʱ���ж�Ƶ��ΪconfigTICK_RATE_HZ */
	SysTick_Config(SystemCoreClock/configTICK_RATE_HZ);									
	
	/* ��ʼ������1 */
	Usart1_Init(115200);  

	/* ����app_task_menu */
	xTaskCreate((TaskFunction_t )app_task_init,  			/* ������ں��� */
			  (const char*    )"app_task_init",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С��512�� */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_init_handle);		/* ������ƿ�ָ�� */ 

	/* ����������� */
	vTaskStartScheduler(); 
	
}

static void app_task_init(void* pvParameters)//�����ʼ��
{
	//LED��ʼ��
	LED_Init();
	//������̳�ʼ��
	key_board_init();
	//oled��ʼ��
	OLED_Init();
	//oled����
	OLED_Clear();
	//������ʼ��
	key_init();	
	//��������ʼ��
	beep_init();
	//ָ��ģ���ʼ��
	fpm_init();
	
	/*����FLASH���������FLASH*/
	FLASH_Unlock();

	/* �����Ӧ�ı�־λ*/  
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
				   FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 	

	//���Ի�ȡ100����¼
	for(i=0;i<100;i++)
	{
		//��ȡ�洢�ļ�¼
		flash_read_record(save_buf,i);
		
		//����¼�Ƿ���ڻ��з��ţ��������򲻴�ӡ���
		if(strstr((const char *)save_buf,"\n")==0)
			break;		
		
	}	
	rec_cnt=i;
	
	//rtc��ʼ��
	rtc_init();
	//�������ڳ�ʼ��
	//Usart3_Init(9600);	
	
	//����������
	g_mutex_oled   = xSemaphoreCreateMutex();
	g_mutex_printf = xSemaphoreCreateMutex();
	
	//������Ϣ����
	g_queue_oled = xQueueCreate(16,sizeof(oled_t));	
	g_queue_rtc  = xQueueCreate(5,64);
	
	//�����¼���־��
	g_event_group_rtc=xEventGroupCreate();
	g_event_group_led=xEventGroupCreate();
	g_event_group_fpm=xEventGroupCreate();
	g_event_group_root=xEventGroupCreate();
	g_event_group_beep=xEventGroupCreate();
	
	printf("-----------------��ӭ���뿼�ڻ�-----------------\n");

	/* ����app_task_oled���� */
	xTaskCreate((TaskFunction_t )app_task_oled,  			/* ������ں��� */
			  (const char*    )"app_task_oled",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С��512�� */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_oled_handle);		/* ������ƿ�ָ�� */ 
			  
	/* ����app_task_keyboard���� */
	xTaskCreate((TaskFunction_t )app_task_keyboard,  		/* ������ں��� */
			  (const char*    )"app_task_keyboard",			/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С��512�� */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_keyboard_handle);	/* ������ƿ�ָ�� */ 
			  
	/* ����app_task_led���� */		  
	xTaskCreate((TaskFunction_t )app_task_led,  			/* ������ں��� */
			  (const char*    )"app_task_led",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_led_handle);		/* ������ƿ�ָ�� */ 

	/* ����app_task_root���� */		  
	xTaskCreate((TaskFunction_t )app_task_root,  			/* ������ں��� */
			  (const char*    )"app_task_root",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_root_handle);		/* ������ƿ�ָ�� */ 
			  
	/* ����app_task_atte���� */		  
	xTaskCreate((TaskFunction_t )app_task_atte,  			/* ������ں��� */
			  (const char*    )"app_task_atte",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_atte_handle);		/* ������ƿ�ָ�� */
			  
	/* ����app_task_beep���� */		  
	xTaskCreate((TaskFunction_t )app_task_beep,  			/* ������ں��� */
			  (const char*    )"app_task_beep",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_beep_handle);		/* ������ƿ�ָ�� */ 

	/* ����app_task_rtc���� */
	xTaskCreate((TaskFunction_t )app_task_rtc,  			/* ������ں��� */
			  (const char*    )"app_task_rtc",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С��512�� */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_rtc_handle);		/* ������ƿ�ָ�� */ 

	/* ����app_task_mod���� */
	xTaskCreate((TaskFunction_t )app_task_mod,  			/* ������ں��� */
			  (const char*    )"app_task_mod",				/* �������� */
			  (uint16_t       )512,  						/* ����ջ��С��512�� */
			  (void*          )NULL,						/* ������ں������� */
			  (UBaseType_t    )5, 							/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_mod_handle);		/* ������ƿ�ָ�� */
 
	/* ����app_task_show���� */
	xTaskCreate((TaskFunction_t )app_task_show,  		/* ������ں��� */
			  (const char*    )"app_task_show",			/* �������� */
			  (uint16_t       )512,  				/* ����ջ��С��512�� */
			  (void*          )NULL,				/* ������ں������� */
			  (UBaseType_t    )5, 					/* ��������ȼ� */
			  (TaskHandle_t*  )&app_task_show_handle);	/* ������ƿ�ָ�� */ 			  

			  
	OLED_ShowString(38,2,(u8 *)"K1",16);
	OLED_ShowCHinese(54,2,0);//��
	OLED_ShowCHinese(70,2,1);//��	
	OLED_ShowString(0,4,(u8 *)"Code Enter ROOT",16);
	OLED_ShowString(64,6,(u8 *)"by",16);
	OLED_ShowCHinese(80,6,26); //��
	OLED_ShowCHinese(96,6,27); //һ
	OLED_ShowCHinese(112,6,28);//��			  
	

	/* ɾ���������� */
	vTaskDelete(NULL);			  
}  



static void app_task_oled(void* pvParameters)//OLED��ʾ
{
	oled_t oled;
	BaseType_t xReturn=pdFALSE;	
	for(;;)
	{
		xReturn = xQueueReceive( g_queue_oled,	/* ��Ϣ���еľ�� */
								&oled, 			/* �õ�����Ϣ���� */
								portMAX_DELAY);	/* �ȴ�ʱ��һֱ�� */
		if(xReturn != pdPASS)
			continue;
		
		switch(oled.ctrl)
		{
			case OLED_CTRL_DISPLAY_ON:
			{
				/* ���� */
				OLED_Display_On();
			}break;

			case OLED_CTRL_DISPLAY_OFF:
			{
				/* ���� */
				OLED_Display_Off();	
			}break;

			case OLED_CTRL_CLEAR:
			{
				/* ���� */
				OLED_Clear();
			}break;

			case OLED_CTRL_SHOW_STRING:
			{
				/* ��ʾ�ַ��� */
				OLED_ShowString(oled.x,
								oled.y,
								oled.str,
								oled.font_size);
				
			}break;

			case OLED_CTRL_SHOW_CHINESE:
			{
				/* ��ʾ���� */
				OLED_ShowCHinese(oled.x,
								oled.y,
								oled.chinese);
			}break;			

			case OLED_CTRL_SHOW_PICTURE:
			{
				/* ��ʾͼƬ */
				OLED_DrawBMP(	oled.x,
								oled.y,
								oled.x+oled.pic_width,
								oled.y+oled.pic_height,
								oled.pic);
			}break;

			default:dgb_printf_safe("[app_task_oled] oled ctrl code is invalid\r\n");	
				break;
		}
	}
}   

static void app_task_keyboard(void* pvParameters)//��Ĥ����
{
	u8   i=0;
	char key_old=0;
	char key_cur=0;
	char password[9]="0";
	char master_xor[9]="0";
	char xor_er[9]="0";
	
	oled_t	 oled;
	uint32_t key_sta=0;
	uint8_t  buf_pw[8]={0};
	uint8_t  buf_order[16]={0};
	
	BaseType_t 	xReturn=pdFALSE;
	
	char master_pw[9]={49,50,51,52,53,54,55,56,'\0'};//  1 2 3 4 5 6 7 8 ASCLL��
	
	xor_encryption(XOR_KEY,master_pw,master_xor,sizeof(master_pw));
	for(;;)
	{
		switch(key_sta)
		{
			case 0://��ȡ���µİ���
			{
				key_cur = get_key_board();	

				if(key_cur != 'N')
				{
					key_old = key_cur;
					key_sta=1;
				}
			}break;
			case 1://ȷ�ϰ��µİ���
			{
				key_cur = get_key_board();	
				
				if((key_cur != 'N') && (key_cur == key_old))
				{
					dgb_printf_safe("KEY %c Down\r\n",key_cur);
					key_sta=2;
					xEventGroupSetBits(g_event_group_beep,0x01);//������
						
					
					if(key_cur == 'C')//����
					{
						dgb_printf_safe("������0\n");
						for(i=0;i<8;i++)
						{
							password[i]=0;
						}
						i = 0;	
					}
					else if(key_cur == '*')//��һλ����
					{
						dgb_printf_safe("������λ\n");
						password[i]=0;
						i--;
					}
					else
					{
						password[i]=key_cur;
						i++;
					}
				}
			}break;
			case 2://��ȡ�ͷŵİ���
			{
				key_cur = get_key_board();	
					
				if(key_cur == 'N')
				{
					key_sta=0;
					key_old =  'N';
				}
			}break;
			default:break;
		}
		if(i == 8)
		{
			i=0;
			
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);	
			
			sprintf((char *)buf_pw,"pw:%s",password);
			dgb_printf_safe("Input %s\r\n",buf_pw);
			/* oled��ʾ */
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=16;
			oled.y=3;
			oled.str=buf_pw;
			oled.font_size=16;
			xReturn = xQueueSend(g_queue_oled,&oled,100);			
			if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			
			xor_decryption(XOR_KEY,master_xor,xor_er,sizeof(master_xor));
			
			if(memcmp(xor_er,password,8) == 0) //����12345678
			{	
				oled.ctrl=OLED_CTRL_CLEAR;					//����
				xQueueSend(g_queue_oled,&oled,100);
				sprintf((char *)buf_order,"Success!");
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=32;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);
				if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				
				xEventGroupSetBits(g_event_group_beep,0x04);//������
				xEventGroupSetBits(g_event_group_led,0x01); //LED
				xEventGroupSetBits(g_event_group_root,0x01); //root
				vTaskDelay(2000);
				
				//����Ա����
				oled.ctrl=OLED_CTRL_CLEAR;					//����
				xQueueSend(g_queue_oled,&oled,100);
				
				sprintf((char *)buf_order,"K2  Add Fpm");
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=16;
				oled.y=0;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);
				if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				vTaskDelay(1000);
				memset(&oled,0,sizeof(oled));
				sprintf((char *)buf_order,"K3  Fpm Num");
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=16;
				oled.y=2;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);
				if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				vTaskDelay(500);
				sprintf((char *)buf_order,"K4  Del All");
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=16;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);
				if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				vTaskDelay(500);
				sprintf((char *)buf_order,"U1/3 Mod Time");
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=8;
				oled.y=6;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);
				if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			
			}else //�������
			{
				xEventGroupSetBits(g_event_group_beep,0x02);//������
				xEventGroupSetBits(g_event_group_led,0x02); //LED
				
				oled.ctrl=OLED_CTRL_CLEAR;					//����
				xQueueSend(g_queue_oled,&oled,100);
				xSemaphoreTake(g_mutex_oled,portMAX_DELAY);
				sprintf((char *)buf_order,"Code Error!");
				/* oled��ʾ */
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=24;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);
				xSemaphoreGive(g_mutex_oled);
				
			}
		}
	}
}   

static void app_task_led(void* pvParameters)//LED����
{
	EventBits_t led_event_bits=0;
	for(;;)
	{
		led_event_bits=xEventGroupWaitBits(g_event_group_led,0xff,pdTRUE,pdFALSE,portMAX_DELAY);
		if(led_event_bits & 0x01)//������ȷ
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_9  | GPIO_Pin_10);
			GPIO_ResetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);			
			vTaskDelay(100);
			GPIO_SetBits(GPIOF,GPIO_Pin_9  | GPIO_Pin_10);
			GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);
		}
		if(led_event_bits & 0x02)//����ʧ��
		{
			GPIO_ResetBits(GPIOF,GPIO_Pin_9  | GPIO_Pin_10);
			GPIO_ResetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);			
			vTaskDelay(500);
			GPIO_SetBits(GPIOF,GPIO_Pin_9  | GPIO_Pin_10);
			GPIO_SetBits(GPIOE,GPIO_Pin_13 | GPIO_Pin_14);
		}

	}
}


static void app_task_root(void* pvParameters)//����Ա
{
	oled_t	 	oled;
	uint8_t 	fmp_error_code;
	uint16_t 	id_total;
	uint8_t  	buf_order[16]={0};
	EventBits_t fpm_event_bits=0;
	EventBits_t root_event_bits=0;
	BaseType_t 	xReturn=pdFALSE;
	
	for(;;)
	{
		/* ����ʵ�����ָ�ơ���ָ֤�ơ���ȡָ�����������ָ�� */
		root_event_bits=xEventGroupWaitBits(g_event_group_root,0xff,pdTRUE,pdFALSE,portMAX_DELAY);
		fpm_event_bits=xEventGroupWaitBits(g_event_group_fpm,0xff,pdTRUE,pdFALSE,portMAX_DELAY);
		
		/* ���ָ�� */
		if((root_event_bits & 0x01) && (fpm_event_bits & 0x02))
		{
			fpm_ctrl_led(FPM_LED_BLUE);
			dgb_printf_safe("--------�뽫��ָ�ŵ�ָ��ģ�鴥����Ӧ��--------\r\n");
			
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);	
			sprintf((char *)buf_order,"Add Fpm");	//��ʾ����
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=32;
			oled.y=2;
			oled.str=buf_order;
			oled.font_size=16;
			xReturn = xQueueSend(g_queue_oled,&oled,100);					
			if(xReturn != pdPASS)
				dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			
			fmp_error_code =fpm_id_total(&id_total);

            if(fmp_error_code == 0)
            {
                dgb_printf_safe("��ȡָ��������%04d\r\n",id_total);
				
				/* ���ָ��*/
				fmp_error_code=fpm_enroll_auto(id_total+1);

				if(fmp_error_code == 0)
				{
					fpm_ctrl_led(FPM_LED_GREEN);					
					
					dgb_printf_safe("�Զ�ע��ָ�Ƴɹ�\r\n");
					
					sprintf((char *)buf_order,"Add Success");	//��ʾ����
					oled.ctrl=OLED_CTRL_SHOW_STRING;
					oled.x=16;
					oled.y=4;
					oled.str=buf_order;
					oled.font_size=16;
					xReturn = xQueueSend(g_queue_oled,&oled,100);				
					if(xReturn != pdPASS)
						dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
					
					xEventGroupSetBits(g_event_group_beep,0x01);//������	
					xEventGroupSetBits(g_event_group_led,0x01); //LED					
				}
				else
				{
					fpm_ctrl_led(FPM_LED_RED);
					
					sprintf((char *)buf_order,"Failed");	//��ʾ����
					oled.ctrl=OLED_CTRL_SHOW_STRING;
					oled.x=48;
					oled.y=4;
					oled.str=buf_order;
					oled.font_size=16;
					xReturn = xQueueSend(g_queue_oled,&oled,100);		
					if(xReturn != pdPASS)
						dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
					
					xEventGroupSetBits(g_event_group_beep,0x02);//������
					xEventGroupSetBits(g_event_group_led,0x02); //LED
				}
				Delay_ms(100);	
				
				fpm_sleep();
				
				Delay_ms(1000);	
            }
		}
		
		/* ��ȡ�û����� */
		if((root_event_bits & 0x01) && (fpm_event_bits & 0x04))
		{
			fpm_ctrl_led(FPM_LED_BLUE);
			
			dgb_printf_safe("\n========================================\r\n");
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);	
			sprintf((char *)buf_order,"Fpm Num");	//��ʾ����
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=32;
			oled.y=2;
			oled.str=buf_order;
			oled.font_size=16;
			xReturn = xQueueSend(g_queue_oled,&oled,100);		
			if(xReturn != pdPASS)
				dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			
			fmp_error_code =fpm_id_total(&id_total);

            if(fmp_error_code == 0)
            {
				fpm_ctrl_led(FPM_LED_GREEN);
				
                dgb_printf_safe("��ȡָ��������%04d\r\n",id_total);
                
				sprintf((char *)buf_order,"Num:%04d",id_total);	//��ʾ����
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=32;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend( 	g_queue_oled,/* ��Ϣ���еľ�� */
										&oled,	/* ���͵���Ϣ���� */
										100);		/* �ȴ�ʱ�� 100 Tick */				
				if(xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				
                xEventGroupSetBits(g_event_group_beep,0x01);//������
				xEventGroupSetBits(g_event_group_led,0x01); //LED
            }
			else
			{
				fpm_ctrl_led(FPM_LED_RED);
				
				sprintf((char *)buf_order,"Failed");	//��ʾ����
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=32;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend( 	g_queue_oled,/* ��Ϣ���еľ�� */
										&oled,	/* ���͵���Ϣ���� */
										100);		/* �ȴ�ʱ�� 100 Tick */				
				if(xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				
				xEventGroupSetBits(g_event_group_beep,0x02);//������
				xEventGroupSetBits(g_event_group_led,0x02); //LED
			}			
			
			Delay_ms(100);		
			
			fpm_sleep();	

			Delay_ms(1000);				
		}	

		
		/* ���ָ�� */
		if((root_event_bits & 0x01) && (fpm_event_bits & 0x08))
		{
			fpm_ctrl_led(FPM_LED_BLUE);
			
			dgb_printf_safe("\n========================================\r\n");
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);	
			sprintf((char *)buf_order,"Del All");	//��ʾ����
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=32;
			oled.y=2;
			oled.str=buf_order;
			oled.font_size=16;
			xReturn = xQueueSend( 	g_queue_oled,/* ��Ϣ���еľ�� */
									&oled,	/* ���͵���Ϣ���� */
									100);		/* �ȴ�ʱ�� 100 Tick */				
			if(xReturn != pdPASS)
				dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			
			fmp_error_code=fpm_empty();
            if(fmp_error_code == 0)
            {
				fpm_ctrl_led(FPM_LED_GREEN);
				
                dgb_printf_safe("���ָ�Ƴɹ�\r\n");
				
				sprintf((char *)buf_order,"Del Success");	//��ʾ����
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=16;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend( 	g_queue_oled,/* ��Ϣ���еľ�� */
										&oled,	/* ���͵���Ϣ���� */
										100);		/* �ȴ�ʱ�� 100 Tick */				
				if(xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
                
                xEventGroupSetBits(g_event_group_beep,0x01);//������
				xEventGroupSetBits(g_event_group_led,0x01); //LED
            }
			else
			{
				fpm_ctrl_led(FPM_LED_RED);
				
				sprintf((char *)buf_order,"Failed");	//��ʾ����
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=32;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend( 	g_queue_oled,/* ��Ϣ���еľ�� */
										&oled,	/* ���͵���Ϣ���� */
										100);		/* �ȴ�ʱ�� 100 Tick */				
				if(xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				
				xEventGroupSetBits(g_event_group_beep,0x02);//������
				xEventGroupSetBits(g_event_group_led,0x02); //LED
			}			
			Delay_ms(100);		
			
			fpm_sleep();	

			Delay_ms(1000);								
		}	
	}
}  

static void app_task_atte(void* pvParameters)//����
{
	oled_t	 	oled;
	uint8_t 	fmp_error_code;
	uint16_t 	id;
	uint8_t  	buf_order[16]={0};
	EventBits_t fpm_event_bits=0;
	BaseType_t 	xReturn=pdFALSE;
	for(;;)
	{
		fpm_event_bits=xEventGroupWaitBits(g_event_group_fpm,0x01,pdTRUE,pdFALSE,portMAX_DELAY);
		/* ˢָ�� */
		if(fpm_event_bits & 0x01)
		{
			fpm_ctrl_led(FPM_LED_BLUE);
			
			dgb_printf_safe("\n============================================\r\n");
			dgb_printf_safe("�뽫��ָ�ŵ�ָ��ģ�鴥����Ӧ��\r\n");
			
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);	
			sprintf((char *)buf_order,"Mod Fpm");	//��ʾ����
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=32;
			oled.y=2;
			oled.str=buf_order;
			oled.font_size=16;
			xReturn = xQueueSend( 	g_queue_oled,
									&oled,	     
									100);		 			
			if(xReturn != pdPASS)
				dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			
			/* ����Ϊ0xFFFF����1:Nƥ�� */
			id = 0xFFFF;
			
			fmp_error_code=fpm_idenify_auto(&id);
            
            if(fmp_error_code == 0)
            {
				fpm_ctrl_led(FPM_LED_GREEN);
				
                dgb_printf_safe("%04d���ڳɹ�!\r\n",id);
					
				sprintf((char *)buf_order,"%04d Attend",id);	//��ʾ����
				sprintf((char *)att_buf,"%04d Attend",id);	
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=16;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);		 			
				if(xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				
                xEventGroupSetBits(g_event_group_beep,0x01);//������
				xEventGroupSetBits(g_event_group_led,0x01); //LED
            }
			else
			{
				fpm_ctrl_led(FPM_LED_RED);
				
				sprintf((char *)buf_order,"Failed");	//��ʾ����
				oled.ctrl=OLED_CTRL_SHOW_STRING;
				oled.x=36;
				oled.y=4;
				oled.str=buf_order;
				oled.font_size=16;
				xReturn = xQueueSend(g_queue_oled,&oled,100);	
				if(xReturn != pdPASS)
					dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
				
				xEventGroupSetBits(g_event_group_beep,0x02);//������
				xEventGroupSetBits(g_event_group_led,0x02); //LED
			}
			
			Delay_ms(100);		
			
			fpm_sleep();	

			Delay_ms(1000);				
			
		}
	}
}	

static void app_task_beep(void* pvParameters)//����������
{
	EventBits_t beep_event_bits=0;
	for(;;)
	{
		beep_event_bits=xEventGroupWaitBits(g_event_group_beep,0xff,pdTRUE,pdFALSE,portMAX_DELAY);
		if(beep_event_bits & 0x01)//��������
		{
			beep_on();Delay_ms(50);beep_off();
		}
		if(beep_event_bits & 0x02)//����ʧ��
		{
			beep_on();Delay_ms(500);beep_off();
		}
		if(beep_event_bits & 0x04)//�����ɹ�
		{
			beep_on();Delay_ms(50);beep_off();Delay_ms(50);
			beep_on();Delay_ms(50);beep_off();
		}
	}
}

static void app_task_rtc(void* pvParameters)//rtc_flash
{
	EventBits_t event_bits_rtc=0;
	
	dgb_printf_safe("--Attendance record num is %d--\r\n",rec_cnt);
			
	for(;;)
	{
		//�ȴ��¼���־����λ
		event_bits_rtc=xEventGroupWaitBits(g_event_group_rtc,0x0f,pdTRUE,pdFALSE,portMAX_DELAY);
		if((event_bits_rtc & 0x01) && (strstr((const char *)att_buf,"Attend")))
		{
			if(rec_cnt<100)
			{
				
//				if(strstr((const char *)att_buf,"Attend"))
//				{
						// ��ȡ����
					RTC_GetDate(RTC_Format_BCD,&RTC_DateStructure);
					
					// ��ȡʱ��
					RTC_GetTime(RTC_Format_BCD, &RTC_TimeStructure);
					
					printf(" 20%02x/%02x/%02x Week:%x %02x:%02x:%02x\r\n", \
										
										RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay,\
										RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
					//��ʽ���ַ���,ĩβ���\r\n��Ϊһ��������ǣ��������Ƕ�ȡ��ʱ������ж�
					sprintf((char *)save_buf,"[%s] 20%02x/%02x/%02x Week:%x %02x:%02x:%02x\r\n", \
									att_buf,\
									RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date,RTC_DateStructure.RTC_WeekDay,\
									RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
					//д����ʪ�ȼ�¼
					if(0==flash_write_record(save_buf,rec_cnt))
					{
						//��ʾ
						dgb_printf_safe("input ---%s",save_buf);					

						//��¼�Լ�1
						rec_cnt++;						
					}
					else
					{
						//���ݼ�¼���㣬��ͷ��ʼ�洢����
						rec_cnt=0;
					}
				//}
				
				
			}
			else
			{
				//����100����¼���ӡ
				dgb_printf_safe("The record has reached 100 and cannot continue writing\r\n");
			}
			memset(att_buf,0,sizeof(att_buf));

		}
		
	}
}   

static void app_task_mod(void* pvParameters)//�޸�ʱ��
{
	uint8_t 	  rt;
	oled_t		  oled;
	char 		  rtc_buf[30]="0";
	uint8_t  	  buf_order[16]={0};
	uint32_t 	  year,month,date,weekday=0;
	uint32_t 	  hour,minute,second=0;
	BaseType_t 	  xReturn=pdFALSE;
	EventBits_t   root_event_bits=0;
	Usart3_Init(9600);
	for(;;)
	{
		root_event_bits=xEventGroupWaitBits(g_event_group_root,0xff,pdTRUE,pdFALSE,portMAX_DELAY);
		xReturn = xQueueReceive( g_queue_rtc,&rtc_buf,portMAX_DELAY);	
		
		dgb_printf_safe("%s\n",rtc_buf);
		
		if((root_event_bits & 0x01) && (strstr((const char *)rtc_buf,"DATE SET")))
		{
			vTaskSuspend(app_task_rtc_handle);
			rt = sscanf((char *)rtc_buf,"DATE SET-20%d-%d-%d-%d#",&year,&month,&date,&weekday);
			RTC_DateStructure.RTC_Year 		= ((year/10) << 4)+year % 10;
			RTC_DateStructure.RTC_Month 	= ((month/10) << 4)+month % 10;
			RTC_DateStructure.RTC_Date 		= ((date/10) << 4)+date % 10;
			RTC_DateStructure.RTC_WeekDay 	= ((weekday/10) <<4) +weekday % 10;
			RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);	
			vTaskResume(app_task_rtc_handle);
			memset((void *)rtc_buf,0,sizeof(rtc_buf));
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);
			
			sprintf((char *)buf_order,"Mod Date Success");
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=0;
			oled.y=3;
			oled.str=buf_order;
			oled.font_size=16;
			xReturn = xQueueSend(g_queue_oled,&oled,100);
			if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			vTaskDelay(1000);
			memset(&oled,0,sizeof(oled));
			
			xEventGroupSetBits(g_event_group_beep,0x04);//������
			xEventGroupSetBits(g_event_group_led,0x01); //LED
				
		}
		else if((root_event_bits & 0x01) && (strstr((const char *)rtc_buf,"TIME SET")))
		{			
			rt = sscanf((char *)rtc_buf,"TIME SET-%d-%d-%d#",&hour,&minute,&second); 
			RTC_TimeStructure.RTC_H12     = RTC_H12_PM;
			RTC_TimeStructure.RTC_Hours   = ((hour/10) << 4)+hour % 10;
			RTC_TimeStructure.RTC_Minutes = ((minute/10) << 4)+minute % 10;
			RTC_TimeStructure.RTC_Seconds = ((second/10) << 4)+second % 10; 
			RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure); 
			memset((void *)rtc_buf,0,sizeof(rtc_buf));
			oled.ctrl=OLED_CTRL_CLEAR;					//����
			xQueueSend(g_queue_oled,&oled,100);
			
			sprintf((char *)buf_order,"Mod Time Success");
			oled.ctrl=OLED_CTRL_SHOW_STRING;
			oled.x=0;
			oled.y=3;
			oled.str=buf_order;
			oled.font_size=16;
			xReturn = xQueueSend(g_queue_oled,&oled,100);
			if(xReturn != pdPASS) dgb_printf_safe("[app_task_rtc] xQueueSend oled string error code is %d\r\n",xReturn);
			vTaskDelay(1000);
			memset(&oled,0,sizeof(oled));
			
			xEventGroupSetBits(g_event_group_beep,0x04);//������
			xEventGroupSetBits(g_event_group_led,0x01); //LED
		}
		else
		{
			dgb_printf_safe("Input error\r\n");
		}
	}
	
}  

static void app_task_show(void* pvParameters)
{
	BaseType_t 	xReturn=pdFALSE;
	char 		flash_buf[64];
	for(;;)
	{
		xReturn = xQueueReceive( g_queue_rtc,&flash_buf,portMAX_DELAY);
		if(strstr((const char *)flash_buf,"show all"))
		{
			for(i=0;i<100;i++)
			{
				//��ȡ�洢�ļ�¼
				flash_read_record(save_buf,i);
				
				//����¼�Ƿ���ڻ��з��ţ��������򲻴�ӡ���
				if(strstr(save_buf,"\n")==0)
					break;		
				
				//��ӡ��¼
				dgb_printf_safe("record[%d]  %s",i,save_buf);
				
			}
			
			//���i����0������û��һ����¼
			if(i==0)
			{
				dgb_printf_safe("There is no record\r\n");
			}
		}
		else if(strstr((char *)flash_buf,"clear"))
		{
			//��������
			flash_erase_record();
			
			dgb_printf_safe("Empty all data records successfully\r\n");
			
			//�����¼����ֵ
			rec_cnt=0;
											
		}
		else 
			continue;
		memset(flash_buf,0,sizeof(flash_buf));
		
	}
}


/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	printf("!![vApplicationStackOverflowHook] %s is StackOverflow\r\n", pcTaskName);
	for( ;; );
}


void vApplicationTickHook( void )
{

}
