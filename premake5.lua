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

    buildoptions { "/we4668" }
    externalanglebrackets "On"
    externalwarnings "Off"

    targetdir "%{prj.location}/Build/Bin/%{cfg.buildcfg}"
    objdir "%{prj.location}/Build/Obj/%{cfg.buildcfg}/%{prj.name}"

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

BinDir = "%{wks.location}/bin/%{cfg.buildcfg}"

group "ThirdParty"
    include "ThirdParty/glfw.lua"
group ""

include "Core"
include "Graphics"
include "Game"
