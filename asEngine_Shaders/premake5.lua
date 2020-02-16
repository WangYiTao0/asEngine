
    project "asEngine_Shaders"
	kind "None"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-obj/" .. outputdir .. "/%{prj.name}")

	files
	{
        "shader/**.hlsl",
        "shader/**.hlsli",
        "src/**.CPP",
    }
    
    includedirs
    {
        "../asEngine/src",
    }

    libdirs
    {
        
    }

    links
    {

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
