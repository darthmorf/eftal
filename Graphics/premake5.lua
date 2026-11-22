project "Graphics"
    location "."
    kind "StaticLib"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src", "../Core/src", "../ThirdParty/DirectX-Headers/include" }

    links { "Core" }
