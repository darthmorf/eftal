project "GLFW"
    location "."
    kind "StaticLib"
    language "C"

    files {
        "glfw/include/GLFW/*.h",
        "glfw/src/internal.h",
        "glfw/src/platform.h",
        "glfw/src/mappings.h",
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/platform.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        "glfw/src/egl_context.c",
        "glfw/src/osmesa_context.c",
        "glfw/src/null_*.h",
        "glfw/src/null_*.c",
        "glfw/src/win32_*.h",
        "glfw/src/win32_*.c",
        "glfw/src/wgl_context.c",
    }

    defines { "_GLFW_WIN32", "_CRT_SECURE_NO_WARNINGS" }

    -- Third-party code: don't hold it to our warning rules.
    removebuildoptions { "/we4668" }
    warnings "Off"
