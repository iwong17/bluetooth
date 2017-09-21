#ifndef _THM3070_H_
#define _THM3070_H_


#ifdef __cplusplus
extern "C"
{
#endif
	
	
	#include "Delay.h"
	#include "SPI.h"
		
	#ifdef _THM3070_C_
	#define EXTERN
	#else
	#define EXTERN extern
	#endif
	
	
	//#define STANDBY	E_GPC,8
	//#define SPIMODE E_GPC,13

	#define DATABUF     0x00
	#define PSEL        0x01
	#define FCONB       0x02
	#define EGT         0x03
	#define CRCSEL      0x04
	#define RSTAT       0x05
	#define SCON        0x06
	#define INTCON      0x07
	#define RSCH        0x08
	#define RSCL        0x09
	#define CRCH        0x0a
	#define CRCL        0x0b
	#define TMRH	    0x0c
	#define TMRL        0x0d
	#define BITPOS      0x0e
	#define SMOD        0x10
	#define PWTH        0x11

	#define EMVEN		0x20
	#define FWTHIGH		0x21
	#define FWTMID		0x22
	#define FWTLOW		0x23
	#define AFDTOFFSET	0x24
	#define EMVERR		0x25
	#define TXFIN		0x26
	#define RERXEN		0x27
	#define TMRM		0x29
	#define TR0MINH		0x2e
	#define TR0MINL		0x2f

	#define RNG_CON		0x30
	#define RNG_STS		0x31
	#define RNG_DATA	0x32
	#define TR1MINH		0x33
	#define TR1MINL		0x34
	#define TR1MAXH		0x35
	#define TR1MAXL		0x36

	//EMVEN bits
	#define FDTPICCINVALIDERR	0x01
	#define FDTPICCNOISEERR 	0x02
	#define FDTTMOUTERR			0x04
	#define TR0ERR				0x08
	#define TR1ERR				0x10 //typeB noise
	#define ALL_NOISEERR		0x20
	#define MSK_NOISEERR		0xED

	#define TXCON				0x40
	#define TXDP1				0x41
	#define TXDP0				0x42
	#define TXDN1				0x43
	#define TXDN0				0x44

	#define RXCON				0x45
	#define	INTER_TRIM1		    0x46
	#define	INTER_TRIM2		    0x47
	#define	INTER_TRIM3		    0x48

	//#define RSTAT bits
	#define FEND        0x01
	#define CRCERR      0x02
	#define TMROVER     0x04
	#define DATOVER     0x08
	#define FERR        0x10
	#define PERR        0x20
	#define CERR        0x40

	#define TYPE_A      0x10
	#define TYPE_B      0x00
	#define ISO15693    0x20
	#define ETK         0x30
	#define MIFARE      0x50
	#define SND_BAUD_106K   0x00
	#define SND_BAUD_212K   0x04
	#define SND_BAUD_424K   0x08
	#define SND_BAUD_848K   0x0c

	#define RCV_BAUD_106K   0x00
	#define RCV_BAUD_212K   0x01
	#define RCV_BAUD_424K   0x02
	#define RCV_BAUD_848K   0x03

	EXTERN void THM_Open_RF(void);
	EXTERN void THM_Close_RF(void);
	EXTERN void THM_Init(void);
	EXTERN signed char THM_Read(unsigned char *buf,unsigned short *len);
	EXTERN char THM_Write(unsigned char *buffer,unsigned short num);
	EXTERN void THM_ClrBuf(void);
	EXTERN void THM_PowerDown(void);
	
	#undef EXTERN
	
#ifdef __cplusplus
}
#endif


#endif
