#pragma once
#include <Windows.h>
#include <synchapi.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

namespace SlimRWLock
{
	typedef struct
	{
		PSRWLOCK lock;
		ULONG thId;
		PINT currentValue;
	} ThreadArg;

	BOOL g_ShouldRun;

	DWORD WINAPI ReaderThreadProc(LPVOID arg)
	{
		ThreadArg* threadArg = (ThreadArg*)arg;
		int cachedValue = -1;
		BOOL first = TRUE;

		while (g_ShouldRun)
		{
			AcquireSRWLockShared(threadArg->lock);
			if (cachedValue != *threadArg->currentValue || first)
			{
				_tprintf(_T("ThreadId %lu ; Current Value has changed %d\n"),
				         threadArg->thId,
				         *threadArg->currentValue);
				Sleep(rand() % 1000 + 1);
			}

			first = FALSE;

			cachedValue = *threadArg->currentValue;
			ReleaseSRWLockShared(threadArg->lock);
		}

		return 0;
	}

	DWORD WINAPI WriterThreadProc(LPVOID arg)
	{
		ThreadArg* threadArg = (ThreadArg*)arg;
		int cachedValue = -1;

		while (g_ShouldRun)
		{
			AcquireSRWLockExclusive(threadArg->lock);
			_tprintf(_T("WriterThreadProc before changing value; Sleeping 3000, Silence for 3 sec\n"));
			Sleep(3000);

			*(threadArg->currentValue) = *threadArg->currentValue + 1;

			ReleaseSRWLockExclusive(threadArg->lock);
			Sleep(20);
		}

		return 0;
	}

	void main()
	{
		SRWLOCK lock;
		InitializeSRWLock(&lock);

		int value = 1;

		ThreadArg* arg1 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg2 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg3 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg4 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg5 = (ThreadArg*)malloc(sizeof(ThreadArg));

		arg1->lock = &lock;
		arg1->thId = 1;
		arg1->currentValue = &value;

		arg2->lock = &lock;
		arg2->thId = 2;
		arg2->currentValue = &value;

		arg3->lock = &lock;
		arg3->thId = 3;
		arg3->currentValue = &value;

		arg4->lock = &lock;
		arg4->thId = 4;
		arg4->currentValue = &value;

		arg5->lock = &lock;
		arg5->thId = 5;
		arg5->currentValue = &value;

		g_ShouldRun = TRUE;

		_tprintf(_T("Press any key to stop\n\n\n\n"));

		HANDLE hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReaderThreadProc, arg1, 0, NULL);
		HANDLE hThread2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReaderThreadProc, arg2, 0, NULL);
		HANDLE hThread3 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReaderThreadProc, arg3, 0, NULL);
		HANDLE hThread4 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReaderThreadProc, arg4, 0, NULL);

		HANDLE hThread5 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WriterThreadProc, arg5, 0, NULL);

		
		getchar();

		g_ShouldRun = FALSE;

		WaitForSingleObject(hThread1, INFINITE);
		WaitForSingleObject(hThread2, INFINITE);
		WaitForSingleObject(hThread3, INFINITE);
		WaitForSingleObject(hThread4, INFINITE);
		WaitForSingleObject(hThread5, INFINITE);
	}
}
