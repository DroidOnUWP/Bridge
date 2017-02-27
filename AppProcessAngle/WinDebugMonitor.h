//////////////////////////////////////////////////////////////
//
//         File: WinDebugMonitor.h
//  Description: Interface of class CWinDebugMonitor
//      Created: 2007-12-6
//       Author: Ken Zhang
//       E-Mail: cpp.china@hotmail.com
//
//////////////////////////////////////////////////////////////

#ifndef __WIN_DEBUG_BUFFER_H__
#define __WIN_DEBUG_BUFFER_H__
#include "pch.h""

class CWinDebugMonitor
{
private:
	enum {
		TIMEOUT_WIN_DEBUG	=	200,
	};

	struct dbwin_buffer
	{
		DWORD   dwProcessId;
		char    data[4096-sizeof(DWORD)];
	};

private:
	HANDLE m_hDBWinMutex;
	HANDLE m_hDBMonBuffer;
	HANDLE m_hEventBufferReady;
	HANDLE m_hEventDataReady;

	HANDLE m_hWinDebugMonitorThread;
	BOOL m_bWinDebugMonStopped;
	struct dbwin_buffer *m_pDBBuffer;

private:
	void Unintialize();

public:
	CWinDebugMonitor();
	~CWinDebugMonitor();

	DWORD Initialize();
	const char* GetDebugString();
	void BufferReady();

	BOOL IsStopped()
	{
		return m_bWinDebugMonStopped;
	}

};

#endif
