workspace "Cooling Force Analysis"
	architecture "x86_64"
	startproject "Cooling Force Analysis"

	configurations
	{
		"Debug",
		"Release",
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	include "Cooling Force Analysis/vendor/tinyfiledialogs"
	include "Cooling Force Analysis/vendor/imgui"
	include "Cooling Force Analysis/vendor/implot"
	include "Cooling Force Analysis/vendor/yaml-cpp"
group ""

group "Core"
	include "Cooling Force Analysis"
group ""