include "dependencies.lua"

project "GPU_Simple"
	kind "ConsoleApp"

	language "C++"
	cppdialect "C++20"

	staticruntime "on"

	targetdir ("%{wks.location}/Binaries/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/Intermediates/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"shaders/*"
	}

	includedirs
	{
		"src",
		"../Utils/src",
	}
	
	libdirs
	{
	}
	
	links
	{
		"Utils"
	}

	LinkOpenCL()
	LinkOpenCV4()
	
	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		symbols "On"
	filter "configurations:Release"
		optimize "On"
	filter "configurations:Dist"
		optimize "Full"