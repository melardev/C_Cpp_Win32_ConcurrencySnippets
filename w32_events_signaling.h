#pragma once
#include <Windows.h>


namespace Win32EventSignaling
{
	char* Response;

	DWORD WINAPI ReadResponse(LPVOID arg)
	{
		HANDLE hEvent = (HANDLE)arg;
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

		const char* message = "Before WaitForSingleObject\n";
		WriteFile(hStdOut, message, strlen(message),
		          NULL, NULL);

		const DWORD dwWaitResult = WaitForSingleObject(hEvent, INFINITE);
		// dwWaitResult == WAIT_OBJECT_0;

		const DWORD bufferLen = strlen(Response);

		WriteFile(hStdOut, Response, bufferLen, NULL, NULL);

		return 0;
	}

	static void main()
	{
		HANDLE hEvent = CreateEventA(NULL, TRUE, FALSE, "Win32EventSignal");
		const HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadResponse, hEvent, 0, NULL);

		Sleep(2000);

		char httpResponse[] = "{\"success\":true}\n";
		Response = httpResponse;
		SetEvent(hEvent);
		WaitForSingleObject(hThread, INFINITE);
	}
}
