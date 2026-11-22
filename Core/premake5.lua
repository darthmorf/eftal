project "Core"
    location "."
    kind "StaticLib"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src" }
