workspace "asEngine"
    architecture "x64"
    targetdir "build"
    startproject "Editor"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to OpenGL-Core
IncludeDir = {}
IncludeDir["ImGui"] = "3rdPart//imgui"

-- Projects
group "Dependencies"
    include "asEngine/3rdPart/imgui"
group ""

include "asEngine"
include "Sandbox"
include "Editor"
include "asEngine_Shaders"

