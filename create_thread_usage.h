#pragma once
#include <Windows.h>
#include <stdio.h>

namespace CreateThreadUsage
{
	void threadProcNoArgs();
	void threadProcWithArgs(int);
	DWORD threadProcWithArgsTraditional(PVOID); // This is actually the signature of THREAD_START_ROUTINE
	void threadFuncWithStringArgs(const char*);

	int main()
	{
		// This snippet is intedeed to be run multiple times so you can see the order of the lines output
		// changing, with that you are warned about race condition bugs. Concurrency is difficult
		// is not only about spawning threads, but also about managing them, and synchronizing them.

		printf("main(): Main Thread Id %lu\n\n", GetCurrentThreadId());
		// We should be using a function that has the signature of THREAD_START_ROUTINE
		// but as you will see, other signatures may work as well
		HANDLE hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)threadProcNoArgs, NULL, NULL, NULL);

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		return 0;
	}

	void threadProcNoArgs()
	{
		const char* message = "CreateThread snippet_1";
		printf("threadProcNoArgs(): Thread Id %lu\n\n", GetCurrentThreadId());
		Sleep(2000);

		char* arg = (char*)malloc(strlen(message) + 1);
		size_t messageLen = strlen(message) + 1; /*Include the null byte str terminator*/
		memcpy_s(arg, messageLen, message, messageLen);

		HANDLE hThread1 = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)threadProcWithArgsTraditional, (LPVOID)arg,
		                               NULL, NULL);

		int value = 2;
		HANDLE hThread2 = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)threadProcWithArgs, (LPVOID)value, NULL,
		                               NULL);

		const char* message2 = "CreateThread snippet_2";

		char* arg2 = (char*)malloc(strlen(message2) + 1);
		size_t messageLen2 = strlen(message2) + 1; /*Include the null byte str terminator*/
		memcpy_s(arg2, messageLen2, message2, messageLen2);
		HANDLE hThread3 = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)threadFuncWithStringArgs, (LPVOID)arg2, NULL,
		                               NULL);

		// Wait for threads to finish
		WaitForSingleObject(hThread1, INFINITE);
		WaitForSingleObject(hThread2, INFINITE);
		WaitForSingleObject(hThread3, INFINITE);


		// Destroy threads
		CloseHandle(hThread1);
		CloseHandle(hThread2);
		CloseHandle(hThread3);
	}


	DWORD threadProcWithArgsTraditional(PVOID arg)
	{
		char* argStr = (char*)arg;
		printf("threadProcWithArgsTraditional(): Thread Id %lu; String arg %s\n\n", GetCurrentThreadId(), argStr);
		Sleep(2000);
		free(argStr);

		return 0;
	}

	void threadProcWithArgs(int value)
	{
		printf("threadProcWithArgs(): Thread Id %lu; Arg integer: %d\n\n", GetCurrentThreadId(), value);
		Sleep(2000);
	}

	void threadFuncWithStringArgs(const char* value)
	{
		printf("threadFuncWithStringArgs(): Thread Id %lu; Arg: %s\n", GetCurrentThreadId(), value);
		free((void*)value);
	}
}
