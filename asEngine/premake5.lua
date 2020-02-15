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
    "src/**.cpp",
--  "3rdPart/bullet/include/**.h",
--  "3rdPart/bullet/include/**.hpp",
--  "3rdPart/bullet/include/**.cpp"
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
    "%{IncludeDir.Bullet}/include",
}

libdirs
{
    "$(VULKAN_SDK)/Lib",
   -- "%{IncludeDir.Bullet}/lib",
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

    libdirs
    {
        "%{IncludeDir.Bullet}/lib/Debug",
    }

    links
    {
        "BulletCollision_vs2010_x64_Debug.lib",
        "BulletDynamics_vs2010_x64_Debug.lib",
        "BulletSoftBody_vs2010_x64_Debug.lib",
        "LinearMath_vs2010_x64_Debug.lib",
    }

filter "configurations:Release"
    defines "AS_RELEASE"
    runtime "Release"
    optimize "on"

    libdirs
    {
        "%{IncludeDir.Bullet}/lib/Release",
    }

    links
    {
        "BulletCollision_vs2010_x64_release.lib",
        "BulletDynamics_vs2010_x64_release.lib",
        "BulletSoftBody_vs2010_x64_release.lib",
        "LinearMath_vs2010_x64_release.lib",
    }