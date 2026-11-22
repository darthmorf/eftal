# TODOs

Tags match the `// TODO_<AREA>` comments in code.

- TODO_ASSERT: add asserts / proper error handling, replacing the temporary `CHECK_HR` macro and bare `return -1`s.
- TODO_LOGGING: add a proper logger in Core (levels, `std::format`-style formatting, output sinks). Under a debugger, route output through `OutputDebugStringA` so it shows in VS's Output window.
- TODO_TEARING: support `DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING` (variable refresh rate / uncapped presentation) when the display supports it.
- TODO_WINDOW_RESIZE: handle window resizing (`ResizeBuffers` and recreating the RTVs after a GPU flush), then re-enable `GLFW_RESIZABLE`.
- TODO_FULLSCREEN: support fullscreen toggling, including Alt+Enter. Currently all DXGI window handling is disabled (`DXGI_MWA_VALID`). Depends on TODO_WINDOW_RESIZE.
- TODO_SYNC: fence waits use `INFINITE`; time out and report a likely device hang instead.
