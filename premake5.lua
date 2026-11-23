workspace "eftal"
    location "."
    configurations { "Debug", "Release" }
    platforms { "x64" }
    startproject "Game"

    language "C++"
    cppdialect "C++20"
    architecture "x86_64"
    system "windows"
    systemversion "latest"

    defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
    buildoptions { "/we4668" }
    externalanglebrackets "On"
    externalwarnings "Off"

    targetdir "%{wks.location}/Bin/%{cfg.buildcfg}"
    objdir "%{wks.location}/Build/Obj/%{cfg.buildcfg}/%{prj.name}"

    filter "kind:StaticLib"
        targetdir "%{wks.location}/Build/Lib/%{cfg.buildcfg}"

    filter "configurations:Debug"
        defines { "_DEBUG", "DEBUG=1", "RELEASE=0" }
        optimize "Off"
        symbols "On"
        runtime "Debug"
        editandcontinue "On"

    filter "configurations:Release"
        defines { "NDEBUG", "DEBUG=0", "RELEASE=1" }
        optimize "Speed"
        symbols "On"
        runtime "Release"

    filter {}

ProjectDir = path.join(_MAIN_SCRIPT_DIR, "Build/Projects")

group "ThirdParty"
    include "ThirdParty/glfw.lua"
group ""

include "Core"
include "Graphics"
include "Game"
