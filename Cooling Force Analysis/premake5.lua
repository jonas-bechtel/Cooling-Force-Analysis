project "Cooling Force Analysis"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files	
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Cooling Force Analysis/src",
		"%{wks.location}/Cooling Force Analysis/vendor/imgui",
		"%{wks.location}/Cooling Force Analysis/vendor/imgui/backends",
		"%{wks.location}/Cooling Force Analysis/vendor/implot",
		"%{wks.location}/Cooling Force Analysis/vendor/tinyfiledialogs",
		"%{wks.location}/Cooling Force Analysis/vendor/ROOT/include",
		"%{wks.location}/Cooling Force Analysis/vendor/yaml-cpp/include/yaml-cpp"
	}

	libdirs 
	{
		"%{wks.location}/Cooling Force Analysis/vendor/ROOT/lib"
	}

	links
	{
		"imgui",
		"implot",
		"tinyfiledialogs",
		"yaml-cpp",
		"libCore",
		"libRIO",
		"libHist",
		"libGpad",
		"libGraf",
		"libGraf3d",
		"libMatrix",
		"libMathCore",
		"libPhysics"
	}

	filter "system:windows"
		systemversion "latest"

		links
		{
			"d3d12",
			"d3dcompiler",
			"dxgi"
		}

	filter "configurations:Debug"
		defines "_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "NDEBUG"
		runtime "Release"
		optimize "on"
