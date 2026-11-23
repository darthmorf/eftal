#pragma once

class Logging
{
public:
	static void Create(bool inAlwaysSpawnCmd = true);
	static void Destroy();

	static void Log(const char* inFmt, ...);

private:
	static bool mIsDebuggerAttached;
	static bool mSpawnedConsole;

	static const size_t BUFFER_SIZE = 512;
};

