#pragma once
#include <Windows.h>
#include <tchar.h>
#include <synchapi.h>


namespace CriticalSectionUsage
{
	CRITICAL_SECTION mutex;

	typedef struct ThreadArg
	{
		CRITICAL_SECTION* mutex;
		ULONG thId;
		const char* message;
	}* PThreadArg;

	DWORD WINAPI ArgOutReflector(LPVOID arg)
	{
		ThreadArg* threadArg = (PThreadArg)arg;

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwTotalWritten = 0, dwWrite = 0;

		BOOL bSuccess = FALSE;

		char message[255];
		sprintf_s(message, 255, "Thread Id %lu; Message: %s\n", threadArg->thId, threadArg->message);
		const DWORD messageLen = strlen(message);


		// Comment this line to see what happens, also comment LeaveCriticalSection
		EnterCriticalSection(threadArg->mutex);


		for (unsigned i = 0; i < 20; i++)
		{
			dwWrite = 0;
			dwTotalWritten = 0;
			while (dwWrite < messageLen)
			{
				bSuccess = WriteFile(hOut, message, messageLen, &dwWrite, NULL);
				if (!bSuccess)
				{
					TCHAR errorMessage[255];
					_stprintf_s(errorMessage, 255, _T("Error Writing to StdOut %lu\n"), threadArg->thId);
					WriteFile(hOut, errorMessage, _tcslen(errorMessage), NULL, NULL);
					ExitProcess(1);
					break;
				}

				dwTotalWritten += dwWrite;
			}

			Sleep(20);
		}

		LeaveCriticalSection(threadArg->mutex);

		return 0;
	}

	static void main()
	{
		InitializeCriticalSection(&mutex);

		ThreadArg* arg1 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg2 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg3 = (ThreadArg*)malloc(sizeof(ThreadArg));

		char msg1[] = "Message 1";
		char msg2[] = "Message 2";
		char msg3[] = "Message 3";

		arg1->thId = 1;
		arg1->mutex = &mutex;
		arg1->message = msg1;

		arg2->thId = 2;
		arg2->mutex = &mutex;
		arg2->message = msg2;

		arg3->thId = 3;
		arg3->mutex = &mutex;
		arg3->message = msg3;


		const HANDLE hThread =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ArgOutReflector, arg1, 0, NULL);

		const HANDLE hThread2 =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ArgOutReflector, arg2, 0, NULL);

		const HANDLE hThread3 =
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ArgOutReflector, arg3, 0, NULL);

		WaitForSingleObject(hThread, INFINITE);
		WaitForSingleObject(hThread2, INFINITE);
		WaitForSingleObject(hThread3, INFINITE);

		DeleteCriticalSection(&mutex);

		free(arg1);
		free(arg2);
		free(arg3);
	}
}
