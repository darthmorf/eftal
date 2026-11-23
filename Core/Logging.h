#pragma once

#include <sal.h> // Needed for _Printf_format_string_ code analysis

enum class LogCat
{
	Misc,
	Assert,
	GLFW,
	DX12,
	Debug,
	Count
};

class Logging
{
public:
	static void Create(bool inSpawnCmd = true);
	static void Destroy();

	static void Log(LogCat inCategory, _Printf_format_string_ const char* inFmt, ...);

private:
	static bool mIsDebuggerAttached;
	static bool mSpawnedConsole;
};
