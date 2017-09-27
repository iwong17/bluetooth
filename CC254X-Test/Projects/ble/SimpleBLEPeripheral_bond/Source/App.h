#ifndef APP_H
#define APP_H

#include "hal_types.h"
#include "hal_mcu.h"

#define LED1 P0_0
#define LED2 P0_1
#define LED3 P1_3

// Function prototypes
extern void GPIOInit(void);
extern void LEDInit(void);

extern void buzzerInit(void);
extern uint8 buzzerStart(uint16 frequency);
extern void buzzerStop(void);

extern void Timer1_Init(void);

extern void Read_Mac(uint8 *ownAddress);
extern void Write_Mac(uint8 *DeviceMAC);

#endif
