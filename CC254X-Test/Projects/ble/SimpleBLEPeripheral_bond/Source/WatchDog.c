#include <ioCC2541.h>

void Init_dog(void) 
{ 
    WDCTL = 0x01;       //定时器间隔选择,间隔约0.25秒
    WDCTL |= 0x08;      //设置看门狗模式
}

void FeetDog(void) 
{ 
    WDCTL = 0xA0;       //清除定时器。当0xA跟随0x5写到这些位，定时器被清除
    WDCTL = 0x50; 
}