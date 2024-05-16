//*****************************************************************************
//*
//*
//*		ComTools.h
//*
//*
//*****************************************************************************
#ifndef 	__COM_TOOLS_H__
#define 	__COM_TOOLS_H__

#include "mcu/mcu.h"

#if MCU_TYPE == PC_EMU && (defined(_WIN32) || defined(__CYGWIN__))


									//  Konstanten f�r Parity
#define 	P_NONE		0			//	0 = kein Parity Bit
#define 	P_EVEN		1			//	1 = gerade
#define 	P_ODD		2			//	2 = ungerade
#define 	P_SPACE		3			//	3 = immer 0
#define 	P_MARK		4			//	4 = immer 1

#define 	D_7BIT		0			// 	7-Databits
#define 	D_8BIT		1			// 	8-Databits

#define 	S_1BIT		0			// 	1   Stopbit
#define 	S_1_5BIT	1			// 	1.5 Stopbits
#define 	S_2BIT		2			// 	2   Stopbits


	void	ComDetectPorts(int *iCount,int *pMode,int iMaxPorts);
	int		ComInit();
	int		ComExit();
	int		ComOpen (unsigned Nr,int Baud,int Parity,int Stopbits,int Databits);
	int		ComClose(unsigned Nr);
//	int		ComRead (unsigned Nr,void *Buffer,int Max);
	int		ComRead (unsigned Nr);
//	int		ComWrite(unsigned Nr,void *Buffer,int Count);
	int		ComWrite(unsigned Nr,int Zeichen);
	int		ComGetReadCount (unsigned Nr);
	int		ComGetWriteCount(unsigned Nr);

#endif


#endif


