//////////////////////////////////////////////////////////////
//
//         File: WinDebugMonitor.cpp
//  Description: Implementation of class CWinDebugMonitor
//      Created: 2007-12-6
//       Author: Ken Zhang
//       E-Mail: cpp.china@hotmail.com
//
//////////////////////////////////////////////////////////////
#include "pch.h""

#include "WinDebugMonitor.h"
#include <stdio.h>

// ----------------------------------------------------------------------------
//  PROPERTIES OF OBJECTS
// ----------------------------------------------------------------------------
//	NAME		|	DBWinMutex		DBWIN_BUFFER_READY		DBWIN_DATA_READY
// ----------------------------------------------------------------------------
//	TYPE		|	Mutex			Event					Event
//	ACCESS		|	All				All						Sync
//	INIT STATE	|	?				Signaled				Nonsignaled
//	PROPERTY	|	?				Auto-Reset				Auto-Reset
// ----------------------------------------------------------------------------

CWinDebugMonitor::CWinDebugMonitor()
{
	if (Initialize() != 0) {
		::OutputDebugString(L"CWinDebugMonitor::Initialize failed.\n");
	}
}

CWinDebugMonitor::~CWinDebugMonitor()
{
	Unintialize();
}

DWORD CWinDebugMonitor::Initialize()
{
	DWORD errorCode = 0;
	BOOL bSuccessful = FALSE;

	SetLastError(0);

	// Mutex: DBWin
	// ---------------------------------------------------------
	m_hDBWinMutex = ::OpenMutex(
		MUTEX_ALL_ACCESS, 
		FALSE, 
		L"DBWinMutex"
		);

	if (m_hDBWinMutex == NULL) {
		errorCode = GetLastError();
		return errorCode;
	}

	// Event: buffer ready
	// ---------------------------------------------------------
	m_hEventBufferReady = ::OpenEvent(
		EVENT_ALL_ACCESS,
		FALSE,
		L"DBWIN_BUFFER_READY"
		);

	if (m_hEventBufferReady == NULL) {
		m_hEventBufferReady = ::CreateEvent(
			NULL,
			FALSE,	// auto-reset
			TRUE,	// initial state: signaled
			L"DBWIN_BUFFER_READY"
			);

		if (m_hEventBufferReady == NULL) {
			errorCode = GetLastError();
			return errorCode;
		}
	}

	// Event: data ready
	// ---------------------------------------------------------
	m_hEventDataReady = ::OpenEvent(
		SYNCHRONIZE,
		FALSE,
		L"DBWIN_DATA_READY"
		);

	if (m_hEventDataReady == NULL) {
		m_hEventDataReady = ::CreateEvent(
			NULL,
			FALSE,	// auto-reset
			FALSE,	// initial state: nonsignaled
			L"DBWIN_DATA_READY"
			);

		if (m_hEventDataReady == NULL) {
			errorCode = GetLastError();
			return errorCode;
		}
	}

	// Shared memory
	// ---------------------------------------------------------
	m_hDBMonBuffer = ::OpenFileMapping(
		FILE_MAP_READ,
		FALSE,
		L"DBWIN_BUFFER"
		);

	if (m_hDBMonBuffer == NULL) {
		m_hDBMonBuffer = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(struct dbwin_buffer),
			L"DBWIN_BUFFER"
			);

		if (m_hDBMonBuffer == NULL) {
			errorCode = GetLastError();
			return errorCode;
		}
	}

	m_pDBBuffer = (struct dbwin_buffer *)::MapViewOfFile(
		m_hDBMonBuffer,
		SECTION_MAP_READ,
		0,
		0,
		0
		);

	if (m_pDBBuffer == NULL) {
		errorCode = GetLastError();
		return errorCode;
	}

	m_bWinDebugMonStopped = false;
	
	return errorCode;
}

void CWinDebugMonitor::Unintialize()
{
	if (m_hWinDebugMonitorThread != NULL) {
		m_bWinDebugMonStopped = TRUE;
		::WaitForSingleObject(m_hWinDebugMonitorThread, INFINITE);
	}

	if (m_hDBWinMutex != NULL) {
		CloseHandle(m_hDBWinMutex);
		m_hDBWinMutex = NULL;
	}

	if (m_hDBMonBuffer != NULL) {
		::UnmapViewOfFile(m_pDBBuffer);
		CloseHandle(m_hDBMonBuffer);
		m_hDBMonBuffer = NULL;
	}

	if (m_hEventBufferReady != NULL) {
		CloseHandle(m_hEventBufferReady);
		m_hEventBufferReady = NULL;
	}

	if (m_hEventDataReady != NULL) {
		CloseHandle(m_hEventDataReady);
		m_hEventDataReady = NULL;
	}
	
	m_pDBBuffer = NULL;
}

const char* CWinDebugMonitor::GetDebugString()
{
	DWORD ret = 0;
	
	// wait for data ready
	ret = ::WaitForSingleObject(m_hEventDataReady, TIMEOUT_WIN_DEBUG);

	if (ret == WAIT_OBJECT_0) {
		return m_pDBBuffer->data;
	}

	return NULL;
}


void CWinDebugMonitor::BufferReady()
{
	// signal buffer ready
	SetEvent(m_hEventBufferReady);

}

