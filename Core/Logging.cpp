#include "Logging.h"

#include <Core/Macros.h>

#include <windows.h>
#include <cstdarg>
#include <cstdio>


// https://talyian.github.io/ansicolors/
#define FG_BLACK	"\x1b[30m"
#define FG_RED		"\x1b[31m"
#define FG_GREEN	"\x1b[32m"
#define FG_YELLOW	"\x1b[33m"
#define FG_BLUE		"\x1b[34m"
#define FG_MAGENTA	"\x1b[35m"
#define FG_CYAN		"\x1b[36m"
#define FG_WHITE	"\x1b[37m"
#define FG_RESET	"\x1b[0m"

#define FG_DEFAULT FG_WHITE

struct CategoryConfig
{
	char mName[5];
	const char* mColour;
};
bool Logging::mIsDebuggerAttached = false;
bool Logging::mSpawnedConsole = false;
static const CategoryConfig gCategoryMapping[] =
{
	{ "MISC"  /* FG_DEFAULT */},
	{ "ASRT", FG_RED },
	{ "GLFW", FG_GREEN },
	{ "DX12", FG_CYAN },
	{ "DBUG", FG_YELLOW },
};
static_assert(ARRAY_COUNT(gCategoryMapping) == static_cast<size_t>(LogCat::Count));

static constexpr int INPUT_BUF_SIZE = 512;
static constexpr int CAT_NAME_LEN = sizeof(CategoryConfig::mName) - 1;
static constexpr int CAT_NAME_BUF_SIZE = CAT_NAME_LEN + 3; // "[" + name + "] "
static constexpr int TOTAL_BUF_SIZE = CAT_NAME_BUF_SIZE + INPUT_BUF_SIZE + 2; // + "\n" + null terminator

void Logging::Create(bool inSpawnCmd)
{
	mIsDebuggerAttached = IsDebuggerPresent();

	// In DEBUG, if we don't have a debugger attached we should ALWAYS spawn a console
#if DEBUG
	if (!mIsDebuggerAttached || inSpawnCmd)
	{
		mSpawnedConsole = AllocConsole();
		
		if (!mSpawnedConsole)
			return; // TODO_ASSERT - FATAL!?

		FILE* f = nullptr;
		freopen_s(&f, "CONOUT$", "w", stdout); // TODO_ASSERT
		SetConsoleOutputCP(CP_UTF8);

		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD mode = 0;
		if (GetConsoleMode(out, &mode))
			SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
#endif
}

void Logging::Log(LogCat inCategory, _Printf_format_string_ const char* inFmt, ...)
{
	const CategoryConfig& categoryMapping = gCategoryMapping[static_cast<int>(inCategory)];

	// Space reserved at the end so a truncated message still gets its newline
	static constexpr int MAX_BODY_LEN = TOTAL_BUF_SIZE - 2;

	char outStr[TOTAL_BUF_SIZE];

	int len = snprintf(outStr, MAX_BODY_LEN + 1, "[%s] ", categoryMapping.mName);
	len = CLAMP(len, 0, MAX_BODY_LEN); // snprintf returns what it would have written, regardless of space

	va_list varArgs;
	va_start(varArgs, inFmt);
	int written = vsnprintf(outStr + len, MAX_BODY_LEN + 1 - len, inFmt, varArgs);
	va_end(varArgs);

	if (written > 0) // TODO_ASSERT, if this fails
		len += written;
	len = CLAMP(len, 0, MAX_BODY_LEN); // snprintf returns what it would have written, regardless of space

	outStr[len] = '\n';
	outStr[len + 1] = '\0';

	if (mSpawnedConsole)
	{
		FILE* stream = stdout;
		fputs(categoryMapping.mColour ? categoryMapping.mColour : FG_DEFAULT, stream);
		fputs(outStr, stream);
		fputs(FG_RESET, stream);
	}

	if (mIsDebuggerAttached)
		OutputDebugStringA(outStr);
}


void Logging::Destroy()
{
#if DEBUG
	if (mSpawnedConsole)
	{
		FreeConsole();
		mSpawnedConsole = false;
	}
#endif

	mIsDebuggerAttached = false;
}