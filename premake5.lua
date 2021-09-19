workspace "Hazel"
	architecture "x64"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir  = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["vendor"] = "Hazel/vendor"
IncludeDir["spdlog"] = "Hazel/vendor/spdlog/include"
IncludeDir["GLFW"] = "Hazel/vendor/GLFW/include"
IncludeDir["Glad"] = "Hazel/vendor/Glad/include"
IncludeDir["ImGui"] = "Hazel/vendor/imgui"
IncludeDir["glm"] = "Hazel/vendor/glm/glm"
IncludeDir["assimp"] = "Hazel/vendor/assimp/include"


include "Hazel/vendor/GLFW"
include "Hazel/vendor/Glad"
include "Hazel/vendor/imgui"


project "Hazel"
	location "Hazel"
	kind "SharedLib"
	language "C++"
	staticruntime "off"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	pchheader "hzpch.h"
	pchsource "Hazel/src/hzpch.cpp"
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{IncludeDir.glm}/**.hpp",
		"%{IncludeDir.glm}/**.inl"
		
	}
	
	includedirs
	{
		"%{prj.name}",
		"%{prj.name}/src",
		"%{IncludeDir.vendor}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.assimp}"
	}
	
	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"assimp-vc142-mtd.lib",
		"opengl32.lib"
	}
	
	libdirs
	{
		"%{IncludeDir.assimp}"
	}
	
	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		
		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}
		
		postbuildcommands
		{
			"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox",
			"{COPY} ../%{IncludeDir.assimp}/assimp-vc142-mtd.dll ../bin/" .. outputdir .. "/Sandbox",
			"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/GUI",
		}
	
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
	
project "GUI"
	location "GUI"
	kind "SharedLib"
	language "C#"
	staticruntime "off"
	clr "Unsafe"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	
	files
	{
		"%{prj.name}/src/**.cs"
		
	}
	
	includedirs
	{
		"Hazel"
	}
	
	links
	{
		"HAZEL"
	}
	
	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL"
		}
		
		postbuildcommands
		{
			"{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox",
		}
	
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
	
project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"Hazel",
		"Hazel/src",
		"%{IncludeDir.vendor}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.assimp}"		
	}
	
	links
	{
		"Hazel",
		"GUI"
	}
	
	
	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"
		
		defines
		{
			"HZ_PLATFORM_WINDOWS",
		}
		
	
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
