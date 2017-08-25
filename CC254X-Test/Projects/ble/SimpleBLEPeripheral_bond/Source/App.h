#ifndef APP_H
#define APP_H

#include "hal_types.h"

// Function prototypes
extern void GPIOInit(void);

extern void buzzerInit(void);
extern uint8 buzzerStart(uint16 frequency);
extern void buzzerStop(void);

extern void GUA_Timer1_Init(void);
#endif
