#include "allhead.h"

/* ������ *//*keyֵ��ԭʼ���롢�������ġ�����*/
void xor_encryption(uint8_t key,char *src,char *dest,uint32_t len)
{
	char *psrc=src;
	char *pdest=dest;

	while(len--)
	{
		*pdest = *psrc ^ key;
		psrc++;
		pdest++;
	}
}

/* ������ *//*keyֵ���������롢ԭʼ���ġ�����*/
void xor_decryption(uint8_t key,char *src,char *dest,uint32_t len)
{

	char *psrc=src;
	char *pdest=dest;

	while(len--)
	{
		*pdest = *psrc ^ key;
		psrc++;
		pdest++;
	}

}
