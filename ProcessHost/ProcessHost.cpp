#include "pch.h"
#include "ProcessHost.h"
#include <Windows.h>
#include <Shellapi.h>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include "android_init.h"
extern "C"
{
#include "../FLinux/src/ntdll.h"
}



void DebugLog(char* pszFormat, ...) {

	static char s_acBuf[2048]; // this here is a caveat!

	va_list args;

	va_start(args, pszFormat);

	vsnprintf(s_acBuf, 2048, pszFormat, args);

	OutputDebugStringA(s_acBuf);

	va_end(args);
}

struct intstack15
{
	intptr_t pos[15];
};


typedef void(*entrypoint_t)(...);

std::wstring atow(const char* str)
{
	size_t len = strlen(str);
	std::wstring ws(len, L' '); // Overestimate number of code points.
	ws.resize(std::mbstowcs(&ws[0], str, len)); // Shrink to fit.
	return ws;
}

int filterException(int code, PEXCEPTION_POINTERS ex) {
	DebugLog("Filtering 0x%x", code);
	DebugLog("Exception code 0x%x", ex->ExceptionRecord->ExceptionCode);
	return EXCEPTION_EXECUTE_HANDLER;
}

void init(const wchar_t* root)
{
	__try
	{
		flinit(root, NULL);
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation())) {
		DebugLog("caught\n");
	}

}

int start()
{
	DebugLog("loading dll\n");

	HMODULE app_process = LoadPackagedLibrary(L"patchoat.dll", 0);

	DebugLog("module handle: 0x%x\n", app_process);

	if (app_process != 0)
	{
		entrypoint_t _module_entry_point = (entrypoint_t)::GetProcAddress(app_process, "_module_entry_point_");


		DebugLog("module entry point 0x%x", _module_entry_point);

		intstack15 stack_params;

		int i = 0;
		/*int j = 0;
		stack_params.pos[i++] = argc;
		for (; j <= argc; i++, j++)
		stack_params.pos[i] = (intptr_t)argv[j];

		stack_params.pos[i++] = 0;

		j = 0;
		for (; j < envc; i++, j++)
		stack_params.pos[i] = (intptr_t)envp[j];*/

		stack_params.pos[i++] = 0;

		_module_entry_point(
#ifdef _M_ARM
			0, 0, 0, 0, //skip register params on ARM, we need copy all params to stack
#endif
			stack_params);

		return 0;

	}

	return -1;

}

int main()
{
	//__debugbreak();
	/*HMODULE mod;
	RTL_USER_PROCESS_INFORMATION process_info;
	NTSTATUS result;

	result = RtlCloneUserProcess(RTL_CLONE_PROCESS_FLAGS_CREATE_SUSPENDED | RTL_CLONE_PROCESS_FLAGS_INHERIT_HANDLES, NULL, NULL, NULL, &process_info);


	if (result == RTL_CLONE_PARENT)
	{
		DebugLog("parent\n");
		ResumeThread(process_info.Thread);
	}
	else if(result == RTL_CLONE_CHILD)
	{
		DebugLog("child\n");
	}
	HANDLE me = GetCurrentProcess();*/


	LPCSTR cmdLine = GetCommandLineA();
	DebugLog("command line: %s\n", cmdLine);


	std::wstring wroot;
	int argc = 0;

	int i = 0;
	int bufferSize = strlen(cmdLine) + 1;
	char* buffer = new char[bufferSize];

	strcpy_s(buffer, bufferSize, cmdLine);

	while (i < bufferSize && buffer[i] == '-' && buffer[i + 1] == '-')
	{
		i += 2; //skip -- 

		if (!strncmp(buffer + i, "root=", 5))
		{
			i += 5;
			int strindex = i;
			while (buffer[i] != ' ' && buffer[i] != 0)
				i++;
			buffer[i++] = 0;
			wroot = atow(buffer + strindex);

		}
		else if (!strncmp(buffer + i, "params=", 7))
		{
			i += 7;
			argc = atoi(buffer + i);
			break;
		}

	}



/*	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, GetCurrentProcessId());

	char sid[100];
	DWORD outLen;


	if (GetTokenInformation(hProcess, TOKEN_INFORMATION_CLASS::TokenOwner, sid, 100, &outLen))
	{
		DebugLog("process handle");
		DebugLog(" %s\n", sid);
	}*/

	init(wroot.c_str());


	return start();
}
