project "Core"
    location (ProjectDir)
    kind "StaticLib"

    files { "**.h", "**.cpp" }
    includedirs { ".." }
