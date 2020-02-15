
    project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-obj/" .. outputdir .. "/%{prj.name}")

	files
	{
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "3rdPart/**.h",
        "3rdPart/**.hpp",
        "3rdPart/**.cpp",
    }
    
    includedirs
    {
        "../asEngine/src",
        "../asEngine/3rdPart/spdlog/include",
        "../asEngine/%{IncludeDir.ImGui}",
        "../asEngine/%{IncludeDir.Bullet}/include",
    }

    libdirs
    {
        "$(VULKAN_SDK)/Lib",
    }
    links
    {
        "asEngine"
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
