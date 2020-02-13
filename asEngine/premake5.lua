project "asEngine"
kind "StaticLib"
language "c++"
cppdialect "c++17"
staticruntime "on"

targetdir("..bin/" .. outputdir .. "/%{prj.name}")
objdir("..bin-obj/" .. outputdir .. "/%{prj.name}")

pchheader "aspch.h"
pchsource "src/aspch.cpp"

--characterset("MBCS")

files
{
    "src/**.h",
    "src/**.cpp"
}

defines
{
    "_CRT_SECURE_NO_WARNINGS"
}

includedirs
{
    "src",
    "$(VULKAN_SDK)/include",
    "3rdPart/spdlog/include",
    "%{IncludeDir.ImGui}",
}

libdirs
{
    "$(VULKAN_SDK)/Lib",
}

links
{
    "vulkan-1",
    "ImGui",
}

filter "system:windows"
    systemversion "latest"

    defines
    {
        "AS_PLATFORM_WINDOWS"
    }

filter "configurations:Debug"
    defines "AS_DEBUG"
    runtime "Debug"
    symbols "on"

filter "configurations:Release"
    defines "AS_RELEASE"
    runtime "Release"
    optimize "on"