workspace "SoftRasterization"
	architecture "x64"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir  = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["vendor"] = "vendor"
IncludeDir["glm"] = "vendor/glm/glm"
IncludeDir["SDL"] = "vendor/SDL/include"
IncludeDir["assimp"] = "vendor/assimp/include"


project "SoftRasterization"
	location "SoftRasterization"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",	
	}
	
	includedirs
	{
		"%{prj.name}",
		"%{prj.name}/src",
		"%{IncludeDir.vendor}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.SDL}",
		"%{IncludeDir.assimp}"
	}
	
	links
	{
		"vendor/SDL/lib/x64/SDL2.lib",
		"assimp-vc142-mtd.lib"
	}
	
	libdirs
	{
		"%{IncludeDir.assimp}"
	}
	
	postbuildcommands
	{
		"{COPY} ../vendor/SDL/lib/x64/SDL2.dll ../bin/" .. outputdir .. "/SoftRasterization",
		"{COPY} ../%{IncludeDir.assimp}/assimp-vc142-mtd.dll ../bin/" .. outputdir .. "/SoftRasterization"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
	
	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "On"
	
	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "On"

