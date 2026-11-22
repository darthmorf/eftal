project "Game"
    location "."
    kind "ConsoleApp"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src", "../Core/src", "../Graphics/src", "../ThirdParty/glfw/include" }

    links { "Core", "Graphics", "GLFW" }

    debugdir (BinDir)
    debugcommand (BinDir .. "/%{cfg.buildtarget.name}")

    postbuildcommands {
        "{MKDIR} \"" .. BinDir .. "\"",
        "{COPYFILE} \"%{cfg.buildtarget.abspath}\" \"" .. BinDir .. "\"",
        "{COPYFILE} \"%{cfg.buildtarget.directory}/%{cfg.buildtarget.basename}.pdb\" \"" .. BinDir .. "\"",
    }
