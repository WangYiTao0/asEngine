
    project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
    }
    
    includedirs
    {
        "../asEngine/src",
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
