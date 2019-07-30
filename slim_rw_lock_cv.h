#pragma once
#include <Windows.h>
#include <synchapi.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

namespace SlimRWLockConditionVariable
{
	typedef struct
	{
		PSRWLOCK lock;
		PCONDITION_VARIABLE cv;
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
			if (first)
			{
				cachedValue = *threadArg->currentValue;
				first = FALSE;
				_tprintf(_T("ThreadId %lu ; First Value %d\n"),
				         threadArg->thId,
				         *threadArg->currentValue);
			}

			// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleepconditionvariablesrw
			while (cachedValue == *threadArg->currentValue)
			{
				SleepConditionVariableSRW(threadArg->cv, threadArg->lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
			}

			cachedValue = *threadArg->currentValue;
			_tprintf(_T("ThreadId %lu ; Current Value has changed %d\n"),
			         threadArg->thId,
			         *threadArg->currentValue);

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
			WakeAllConditionVariable(threadArg->cv);
			Sleep(100);
		}

		return 0;
	}

	void main()
	{
		SRWLOCK lock;
		CONDITION_VARIABLE cv;
		InitializeSRWLock(&lock);
		InitializeConditionVariable(&cv);

		int value = 1;

		ThreadArg* arg1 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg2 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg3 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg4 = (ThreadArg*)malloc(sizeof(ThreadArg));
		ThreadArg* arg5 = (ThreadArg*)malloc(sizeof(ThreadArg));

		arg1->lock = &lock;
		arg1->cv = &cv;
		arg1->thId = 1;
		arg1->currentValue = &value;

		arg2->lock = &lock;
		arg2->cv = &cv;
		arg2->thId = 2;
		arg2->currentValue = &value;

		arg3->lock = &lock;
		arg3->cv = &cv;
		arg3->thId = 3;
		arg3->currentValue = &value;

		arg4->lock = &lock;
		arg4->cv = &cv;
		arg4->thId = 4;
		arg4->currentValue = &value;

		arg5->lock = &lock;
		arg5->cv = &cv;
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
