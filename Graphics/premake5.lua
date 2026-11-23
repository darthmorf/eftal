project "Graphics"
    location (ProjectDir)
    kind "StaticLib"

    files { "**.h", "**.cpp" }
    includedirs { "..", "../ThirdParty/DirectX-Headers/include" }

    links { "Core" }
