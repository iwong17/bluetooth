#ifndef DELAY_H
#define DELAY_H

#include "hal_types.h"

// Function prototypes
extern void Timer1_Init(unsigned int time);
extern void Delay_ms11(unsigned int time);
extern void Delay_ms(unsigned int time);
extern void Delay_us(unsigned int time);


#endif