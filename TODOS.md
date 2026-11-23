# TODOs

Tags match the `// TODO_<AREA>` comments in code.

- TODO_ASSERT: add asserts / proper error handling, replacing the temporary `CHECK_HR` macro and bare `return -1`s.
- TODO_LOGGING: finish the logger in `Core/Logging`. Current state: printf-style `Logging::Log(LogCat, fmt, ...)` into a fixed stack buffer, `[NAME] ` category prefix, newline appended by `Log`. Writes to `OutputDebugStringA` when a debugger is attached, and to a spawned console (Debug only, ANSI colour per category) otherwise or when `Create(true)`.
  - File output: always write a log file. Decide location (`Bin/<Config>/` vs `%LOCALAPPDATA%\eftal\`) and flush policy (at least flush on errors so crashes don't lose them).
  - Release builds currently log nowhere unless a debugger is attached; file output fixes this.
  - Buffered output: append formatted lines to a memory buffer and flush to the sinks in batches (per frame / when full / immediately on errors), possibly on a background thread later. Console and `OutputDebugStringA` writes are slow, so avoid per-line OS calls in volume.
  - Severity levels (Info/Warning/Error) alongside or instead of categories; warnings/errors to `stderr` and always flushed.
  - Optional `file(line):` prefix (macro) for errors so lines are double-clickable in VS's Output window.
  - Route D3D12 debug-layer messages into the logger via `ID3D12InfoQueue1::RegisterMessageCallback` so they reach the console/file without a debugger.
- TODO_MATHS_LIB: add a maths library (no stdlib). Replace the `CLAMP` macro in `Core/Macros.h` with a constexpr template function so arguments are evaluated once and type-checked; add min/max etc. alongside it.
- TODO_TEARING: support `DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING` (variable refresh rate / uncapped presentation) when the display supports it.
- TODO_WINDOW_RESIZE: handle window resizing (`ResizeBuffers` and recreating the RTVs after a GPU flush), then re-enable `GLFW_RESIZABLE`.
- TODO_FULLSCREEN: support fullscreen toggling, including Alt+Enter. Currently all DXGI window handling is disabled (`DXGI_MWA_VALID`). Depends on TODO_WINDOW_RESIZE.
- TODO_SYNC: fence waits use `INFINITE`; time out and report a likely device hang instead.
