project "Game"
    location (ProjectDir)
    kind "WindowedApp"
    entrypoint "mainCRTStartup"

    files { "**.h", "**.cpp" }
    includedirs { "..", "../ThirdParty/glfw/include", "../ThirdParty/DirectX-Headers/include" }

    links { "Core", "Graphics", "GLFW", "d3d12", "dxgi", "dxguid" }

    debugdir "%{cfg.targetdir}"
