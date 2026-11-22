project "Game"
    location "."
    kind "ConsoleApp"

    files { "src/**.h", "src/**.cpp" }
    includedirs { "src", "../Core/src", "../Graphics/src" }

    links { "Core", "Graphics" }

    debugdir (BinDir)
    debugcommand (BinDir .. "/%{cfg.buildtarget.name}")

    postbuildcommands {
        "{MKDIR} \"" .. BinDir .. "\"",
        "{COPYFILE} \"%{cfg.buildtarget.abspath}\" \"" .. BinDir .. "\"",
        "{COPYFILE} \"%{cfg.buildtarget.directory}/%{cfg.buildtarget.basename}.pdb\" \"" .. BinDir .. "\"",
    }
