#include <ioCC2541.h>

void Init_dog(void) 
{ 
    WDCTL  = 0x00;      //��ʱ�����ѡ��,���Ϊ1��
    WDCTL |= 0x08;      //���ÿ��Ź�ģʽ
}

void FeetDog(void) 
{ 
    WDCTL |= 0xA0;      //�����ʱ������0xA����0x5д����Щλ����ʱ�������
    WDCTL |= 0x50; 
}