project "Graphics"
    location "."
    kind "StaticLib"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src", "../Core/src" }

    links { "Core" }
