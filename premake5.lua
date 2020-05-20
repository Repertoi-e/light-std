newoption {
    trigger = "no-crt",
    description = "Disable linking with the Visual C++ Runtime Library on Windows."
}

newoption {
	trigger = "python",
	value = "path",
	description = "Include and link to Python 3.7 C API searching for files in the specified path. e.g. C:/ProgramData/Anaconda3/ (we use C:/ProgramData/Anaconda3/include etc...)"
}

workspace "light-std"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }

function common_settings()
    architecture "x64"

	language "C++"
	cppdialect "C++17"

    rtti "Off"
	characterset "Unicode"
	
	if not _OPTIONS["python"] then
		exceptionhandling "Off"
	end
	
	editandcontinue "Off"

    defines "_HAS_EXCEPTIONS=0"

	includedirs { "%{prj.name}/src" }

	filter "system:windows"
		excludes "%{prj.name}/src/posix_*.cpp"
		systemversion "latest"
		defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS" }
		buildoptions { "/utf-8" }
		links { "dwmapi.lib", "dbghelp.lib" }

	-- Exclude directx files on non-windows platforms since they would cause a compilation failure
	filter "not system:windows"
		excludes  { "%{prj.name}/src/d3d_*.h", "%{prj.name}/src/d3d_*.cpp" }

    filter { "system:windows", "not options:no-crt" }
        staticruntime "On"
        excludes "%{prj.name}/src/windows_no_crt.cpp"

    filter { "system:windows", "options:no-crt" }
        defines "BUILD_NO_CRT"
        flags { "NoRuntimeChecks", "NoBufferSecurityCheck" }
        buildoptions { "/Gs9999999" }
    filter { "system:windows", "options:no-crt", "not kind:StaticLib" }
        linkoptions { "/nodefaultlib", "/subsystem:windows", "/stack:\"0x100000\",\"0x100000\"" }
        links { "kernel32", "shell32", "winmm", "ole32" }
        flags { "OmitDefaultLibrary" }
    filter { "system:windows", "options:no-crt", "kind:SharedLib" }
        entrypoint "main_no_crt_dll"
    filter { "system:windows", "options:no-crt", "kind:ConsoleApp or WindowedApp" }
        entrypoint "main_no_crt"

	filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
		buildoptions { "/FS" }
    filter "configurations:Release"
        defines "RELEASE"
		optimize "On"
        symbols "On"
		buildoptions { "/FS" }
    filter "configurations:Dist"
        defines "DIST"
		optimize "Full"
		floatingpoint "Fast"
	filter {}
end


outputFolder = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "light-std"
	location "%{prj.name}"
	kind "StaticLib"

	targetdir("bin/" .. outputFolder .. "/%{prj.name}")
	objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.inc",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp"
	}

	filter {}

	pchheader "pch.h"
	pchsource "%{prj.name}/src/pch.cpp"
	forceincludes { "pch.h" }

	common_settings()

project "test-suite"
	location "%{prj.name}"
	kind "ConsoleApp"

	targetdir("bin/" .. outputFolder .. "/%{prj.name}")
	objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	links { "light-std" }
	includedirs { "light-std/src" }

	pchheader "test.h"
	pchsource "%{prj.name}/src/test.cpp"
	forceincludes { "test.h" }

	common_settings()

project "benchmark"
	location "%{prj.name}"
	kind "ConsoleApp"

	targetdir("bin/" .. outputFolder .. "/%{prj.name}")
	objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	links { "light-std" }
	includedirs { "light-std/src", "%{prj.name}/vendor/benchmark/include" }

	common_settings()
	
	filter "system:windows"
		links { "shlwapi.lib", "%{prj.location}/vendor/benchmark/lib/Windows/%{cfg.buildcfg}/benchmark.lib" }
	filter "system:linux"
        links { "benchmark" }

project "game"
	location "%{prj.name}"
	kind "ConsoleApp"

	targetdir("bin/" .. outputFolder .. "/%{prj.name}")
	objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	excludes { 
		"%{prj.name}/src/cars/**.h", 
		"%{prj.name}/src/cars/**.cpp",
		"%{prj.name}/src/physics/**.h", 
		"%{prj.name}/src/physics/**.cpp"
	}

	links { "light-std" }
	includedirs { "light-std/src" }

	dependson { "cars", "physics" }

	pchheader "game.h"
	pchsource "game/src/game.cpp"
	forceincludes { "game.h" }

	common_settings()
	
	filter "system:windows"
		links { "dxgi.lib", "d3d11.lib", "d3dcompiler.lib", "d3d11.lib", "d3d10.lib" }

project "cars"
	location "game"
	kind "SharedLib"

	targetdir("bin/" .. outputFolder .. "/game")
	objdir("bin-int/" .. outputFolder .. "/game/%{prj.name}")

	files {
		"game/src/cars/**.h", 
		"game/src/cars/**.cpp"
	}

	defines { "LE_BUILDING_GAME" }

	links { "light-std" }
	includedirs { "light-std/src", "game/src" }

	includedirs { "game/src" }
	pchheader "game.h"
	pchsource "game/src/game.cpp"
	forceincludes { "game.h" }

	common_settings()

	-- Unique PDB name each time we build (in order to support debugging while hot-swapping the game dll)
	filter "system:windows"
		symbolspath '$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString("ddMMyyyy_HHmmss_fff")).pdb'
		links { "dxgi.lib", "d3d11.lib", "d3dcompiler.lib", "d3d11.lib", "d3d10.lib" }


project "physics"
	location "game"
	kind "SharedLib"

	targetdir("bin/" .. outputFolder .. "/game")
	objdir("bin-int/" .. outputFolder .. "/game/%{prj.name}")

	files {
		"game/src/physics/**.h", 
		"game/src/physics/**.cpp"
	}

	defines { "LE_BUILDING_GAME" }

	links { "light-std" }
	includedirs { "light-std/src", "game/src" }

	if _OPTIONS["python"] then
		py = path.getabsolute(_OPTIONS["python"])
		includedirs { py  .. "/include" }
		links { py .. "/libs/python37.lib" }
	end

	includedirs { "game/src" }
	pchheader "pch.h"
	pchsource "game/src/physics/pch.cpp"
	forceincludes { "pch.h" }

	common_settings()

	-- Unique PDB name each time we build (in order to support debugging while hot-swapping the game dll)
	filter "system:windows"
		symbolspath '$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString("ddMMyyyy_HHmmss_fff")).pdb'
		links { "dxgi.lib", "d3d11.lib", "d3dcompiler.lib", "d3d11.lib", "d3d10.lib" }
