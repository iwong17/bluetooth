#ifndef APP_H
#define APP_H

#include "hal_types.h"

// Function prototypes
extern void GPIOInit(void);

extern void buzzerInit(void);
extern uint8 buzzerStart(uint16 frequency);
extern void buzzerStop(void);

extern void Timer1_Init(void);

extern void Read_Mac(uint8 *ownAddress);

void Delay_ms(unsigned short time);
#endif
