//*****************************************************************************
//*
//*
//*		ComTools.cpp
//*
//*
//*****************************************************************************
//
//	(C) Copyright Anton Zechner 2007
//
#include "mcu/mcu.h"
#if MCU_TYPE == PC_EMU && (defined(_WIN32) || defined(__CYGWIN__))
#include	<windows.h>
#include	<memory.h>
#include	"mcu_uart_tools.h"
#include 	<stdio.h>

#define 	MAX_COM_PORTS	30

#ifdef		UNICODE
#define 	Z(a)		L##a
#else
#define 	Z(a)		a
#endif


static HANDLE	hComFile[MAX_COM_PORTS];
static BOOL		bIsOpen	[MAX_COM_PORTS];

//*****************************************************************************
//*
//*		ComInit
//*
//*****************************************************************************
int ComInit()
{
	return 1;
}

//*****************************************************************************
//*
//*		ComExit
//*
//*****************************************************************************
int ComExit()
{
	int	i;

	for(i=0;i<MAX_COM_PORTS;i++)
	{
		if(!bIsOpen[i])continue;
		ComClose(i);
	}

	return 1;
}

//*****************************************************************************
//*
//*		ComOpen
//*
//*****************************************************************************
//	�ffnet eine serielle Verbindung
//	Nr			: Ist die Nummer des Com-Ports (1=COM1 2=COM2 ...)
//	Baud		: Ist die Bautrate
//	Parity		: 0 = kein Parity Bit
//				  1 = gerade
//				  2 = ungerade
//				  3	= immer 0
//				  4 = immer 1
//	Stopbits	: 0 = Ein Stopbit
//				  1 = Ein/einhalb Stopbits
//				  2 = Zwei Stopbits
//	Bits		: 0 = 7 Datenbits
//				  1 = 8 Datenbits
//				  7 = 7 Datenbits
//				  8 = 8 Datenbits
//	Ergibt 1 wenn eine Schnittstelle ge�ffnet wurde
int ComOpen(unsigned Nr,int Baud,int Parity,int Stopbits,int Databits)
{
	static const int	iPMode[]={NOPARITY,EVENPARITY,ODDPARITY,SPACEPARITY,MARKPARITY};
	static const int	iSMode[]={ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS,ONESTOPBIT};
	TCHAR				cName[20];
	HANDLE				hFile;
	COMMTIMEOUTS		sTo;
	DCB					sDcb;



	if(Nr>=MAX_COM_PORTS)return 0;
	if(bIsOpen[Nr])return 0;

	sprintf(cName, "\\\\.\\COM%d", Nr+1);

	hFile= CreateFile(cName,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		hFile=0;
		return 0;
	}

	if(Databits==7)Databits=0;


	memset(&sDcb,0,sizeof(sDcb));
	sDcb.DCBlength=sizeof(sDcb);
	sDcb.BaudRate     	= Baud;
	sDcb.fParity      	= (Parity!=0)? TRUE:FALSE;
	sDcb.fBinary      	= TRUE;
	sDcb.Parity       	= iPMode[Parity];
	sDcb.StopBits     	= iSMode[Stopbits&3];
	sDcb.fOutxCtsFlow 	= FALSE;
	sDcb.fOutxDsrFlow 	= FALSE;
	sDcb.fDtrControl	= DTR_CONTROL_ENABLE;
	sDcb.fRtsControl	= RTS_CONTROL_ENABLE;
	sDcb.fDsrSensitivity= FALSE;
	sDcb.fAbortOnError	= FALSE;
	sDcb.ByteSize     	= (Databits)? 8:7;

	if(!SetCommState(hFile,&sDcb))
	{
		CloseHandle(hFile);
		return 0;
	}


	sTo.ReadIntervalTimeout		   = MAXDWORD; 		// 0 ms Read-Tomeout
	sTo.ReadTotalTimeoutMultiplier = 0;
	sTo.ReadTotalTimeoutConstant   = 0;
	sTo.WriteTotalTimeoutMultiplier= 12000/Baud+1;	// ? ms Write timeout per byte
	sTo.WriteTotalTimeoutConstant  = sTo.WriteTotalTimeoutMultiplier+1;
	if(!SetCommTimeouts((HANDLE)hFile,&sTo))
	{
		CloseHandle(hFile);
		return 0;
	}


	hComFile[Nr]=hFile;
	bIsOpen [Nr]=TRUE;


	return 1;
}



//*****************************************************************************
//*
//*		ComClose
//*
//*****************************************************************************
//	Schlie�t eine serielle Verbindung
//	Nr	: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt 1 wenn eine Schnittstelle geschlossen wurde
int	ComClose(unsigned Nr)
{

	if(Nr>=MAX_COM_PORTS)return 0;
	if(!bIsOpen[Nr])return 0;

	CloseHandle(hComFile[Nr]);
	hComFile[Nr]=0;
	bIsOpen [Nr]=FALSE;


	return 1;
}


//*****************************************************************************
//*
//*		ComDetectPorts
//*
//*****************************************************************************
//	Speichert in iCount die Anzahl der gefundenen COM Ports
//	In pMode werden die Portzust�nte gespeichert
//		0 = Nicht vorhanden
//		1 = Vorhanden
//		2 = Vorhanden und von einem anderen Programm benutzt
//		pMode[0] f�r COM1
//		pMode[1] f�r COM2
//		...
//	iMaxPorts ist die Anzahl der Ports die gescannt werden.
/*
void ComDetectPorts(int &iCount,int *pMode,int iMaxPorts)
{
int		i;
TCHAR	cName[]=Z("\\\\.\\COM1");
HANDLE	hCom;



	for(i=0;i<iMaxPorts;i++)
		{
		if(i<MAX_COM_PORTS && bIsOpen[i])
			{
			pMode[i]=1;
			iCount++;
			continue;
			}

		cName[7]='1'+i;
		hCom=CreateFile(cName,GENERIC_WRITE|GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
		if(hCom==INVALID_HANDLE_VALUE)
			{
			if(GetLastError()==ERROR_FILE_NOT_FOUND)
				{
				pMode[i]=0;
				continue;
				}
			else{
				pMode[i]=2;
				iCount++;
				continue;
				}
			}

		CloseHandle(hCom);
		pMode[i]=1;
		iCount++;
		}

}
 */

//*****************************************************************************
//*
//*		ComRead
//*
//*****************************************************************************
//	Ein Zeichen lesen
//	Nr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt -1 wenn nichts gelesen wurde sonst das Zeichen
int ComRead(unsigned Nr)
{
	unsigned char 	c;
	DWORD			dwCount;


	if(Nr>=MAX_COM_PORTS)return -1;
	if(!bIsOpen[Nr])return -1;

	if(!ReadFile(hComFile[Nr],&c,1,&dwCount,0))return -1;
	if(dwCount!=1)return -1;


	return c;
}

//*****************************************************************************
//*
//*		ComRead
//*
//*****************************************************************************
//	Mehrere Zeichen lesen
//	Nr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Buffer	: Buffer in dem die Zeichen gespeichert werden
//	Max		: Maximale Anzahl der zu lesenden Zeichen
//	Ergibt die Anzahl der gelesenen Zeichen
/*
int ComRead(unsigned Nr,void *Buffer,int Max)
{
DWORD	dwCount;


	if(Nr>=MAX_COM_PORTS)return 0;
	if(!bIsOpen[Nr])return 0;

	ReadFile(hComFile[Nr],Buffer,Max,&dwCount,0);

return dwCount;
}
 */

//*****************************************************************************
//*
//*		ComWrite
//*
//*****************************************************************************
//	Ein Zeichen senden
//	Nr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Zeichen	: Ist das Zeichen das gesendet werden soll.
//	Ergibt die Anzahl der gesendeten Zeichen
int ComWrite(unsigned Nr,int Zeichen)
{
	DWORD			dwCount;


	if(Nr>=MAX_COM_PORTS)return 0;
	if(!bIsOpen[Nr])return 0;

	WriteFile(hComFile[Nr],&Zeichen,1,&dwCount,0);


	return dwCount;
}

//*****************************************************************************
//*
//*		ComWrite
//*
//*****************************************************************************
//	Mehrere Zeichen schreiben
//	Nr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Buffer	: Buffer in dem die Zeichen gespeichert werden
//	Count	: Anzahl der zu sendenden Zeichen
//	Ergibt die Anzahl der gesendeten Zeichen
/*
int	ComWrite(unsigned Nr,void *Buffer,int Count)
{
DWORD			dwCount;


	if(Nr>=MAX_COM_PORTS)return 0;
	if(!bIsOpen[Nr])return 0;

	WriteFile(hComFile[Nr],Buffer,Count,&dwCount,0);

return dwCount;
}
 */

//*****************************************************************************
//*
//*		ComGetReadCount
//*
//*****************************************************************************
//	Ergibt die Anzahl der Bytes die im Lesepuffer der Schnittstelle sind
//	Nr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt die Anzahl der Bytes im Buffer
int ComGetReadCount(unsigned Nr)
{
	COMSTAT		sComStat = {0};
	DWORD		dwErrorFlags = 0;

	if(Nr>=MAX_COM_PORTS)
		return 0;

	if(!bIsOpen[Nr])
		return 0;

	if(!ClearCommError(hComFile[Nr], &dwErrorFlags, &sComStat))
	{
		return 0;
	}

	return sComStat.cbInQue;
}

//*****************************************************************************
//*
//*		ComGetWriteCount
//*
//*****************************************************************************
//	Ergibt die Anzahl der Bytes die im Schreibpuffer der Schnittstelle sind
//	Nr		: Ist die Nummer des Com-Ports (0=COM1 1=COM2 ...)
//	Ergibt die Anzahl der Bytes im Buffer
int ComGetWriteCount(unsigned Nr)
{
	COMSTAT		sComStat;
	DWORD		dwErrorFlags;


	if(Nr>=MAX_COM_PORTS)return 0;
	if(!bIsOpen[Nr])return 0;

	dwErrorFlags=0;

	if(!ClearCommError(hComFile[Nr], &dwErrorFlags, &sComStat))return 0;

	return sComStat.cbOutQue;
}

#endif
