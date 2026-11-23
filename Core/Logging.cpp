#include "Logging.h"

#include <windows.h>
#include <cstdio>


bool Logging::mIsDebuggerAttached = false;
bool Logging::mSpawnedConsole = false;


void Logging::Create(bool inAlwaysSpawnCmd)
{
	mIsDebuggerAttached = IsDebuggerPresent();

	// In DEBUG, if we don't have a debugger attached we should spawn a console instead
#if DEBUG
	if (!mIsDebuggerAttached || inAlwaysSpawnCmd)
	{
		mSpawnedConsole = AllocConsole();
		
		if (!mSpawnedConsole)
			return; // TODO_ASSERT - FATAL!?

		FILE* f = nullptr;
		freopen_s(&f, "CONOUT$", "w", stdout);
		freopen_s(&f, "CONOUT$", "w", stderr);
		SetConsoleOutputCP(CP_UTF8);

		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode = 0;
		if (GetConsoleMode(out, &mode))
			SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
#endif
}


void Logging::Log(const char* inFmt, ...)
{
	char outStr[BUFFER_SIZE];
	outStr[0] = '\n';

	va_list argptr;
	va_start(argptr, inFmt);
	vsnprintf(outStr + 1, BUFFER_SIZE, inFmt, argptr); // TODO_ASSERT, if this fails
	va_end(argptr);

	OutputDebugStringA(outStr);

	FILE* stream = stdout;
	fputs(outStr, stream);
}


void Logging::Destroy()
{
#if DEBUG
	if (mSpawnedConsole)
		FreeConsole();
#endif
}