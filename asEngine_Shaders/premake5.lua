
    project "asEngine_Shaders"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-obj/" .. outputdir .. "/%{prj.name}")

	files
	{
        "shaders/**.hlsl",
        "shaders/**.hlsli",
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


-- Shader Compiler Setting--------------------------
    shaderassembler("AssemblyCode")
    --shaderoptions instead of shaderincluders
    shaderoptions "./asEngine/src/Graphics/GPUMapping"
    filter { "files:**.hlsli" }
    flags "ExcludeFromBuild"
    
    
    filter { "files:**.hlsl" }
    flags "ExcludeFromBuild"
    shadermodel "5.0"
    shaderobjectfileoutput "shaderCSO/%{file.basename}.cso"
    shaderassembleroutput "shaderASM/%{file.basename}.asm"

    filter { "files:**VS.hlsl" }
    removeflags "ExcludeFromBuild"
    shadertype "Vertex"
    shaderentry "main"

    filter { "files:**PS.hlsl" }
    removeflags "ExcludeFromBuild"
    shadertype "Pixel"
    shaderentry "main"

    filter { "files:**HS.hlsl" }
    removeflags "ExcludeFromBuild"
    shadertype "Hull"
    shaderentry "main"

    filter { "files:**DS.hlsl" }
    removeflags "ExcludeFromBuild"
    shadertype "Domain"
    shaderentry "main"

    filter { "files:**GS.hlsl" }
    removeflags "ExcludeFromBuild"
    shadertype "Geometry"
    shaderentry "main"

    filter { "files:**CS.hlsl" }
    removeflags "ExcludeFromBuild"
    shadertype "Compute"
    shaderentry "main"

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
