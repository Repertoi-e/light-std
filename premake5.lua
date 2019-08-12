newoption {
    trigger = "no-crt",
    description = "Disable linking with the Visual C++ Runtime Library on Windows."
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
    exceptionhandling "Off"
	editandcontinue "Off"

    defines "_HAS_EXCEPTIONS=0"

	filter "system:windows"
		excludes "%{prj.name}/src/posix_*.cpp"
		systemversion "latest"
		defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN" }
		buildoptions { "/utf-8" }

    filter { "system:windows", "not options:no-crt" }
        staticruntime "On"
        excludes "%{prj.name}/src/windows_no_crt.cpp"

    filter { "system:windows", "options:no-crt" }
        defines "BUILD_NO_CRT"
        flags { "NoRuntimeChecks", "NoBufferSecurityCheck" }
        buildoptions { "/Gs9999999" }
    filter { "system:windows", "options:no-crt", "not kind:StaticLib" }
        linkoptions { "/nodefaultlib", "/subsystem:windows", "/stack:\"0x100000\",\"0x100000\"" }
        links { "Kernel32", "Shell32", "Winmm" }
        flags { "OmitDefaultLibrary" }
    filter { "system:windows", "options:no-crt", "kind:SharedLib" }
        entrypoint "main_no_crt_dll"
    filter { "system:windows", "options:no-crt", "kind:ConsoleApp or WindowedApp" }
        entrypoint "main_no_crt"

	filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
    filter "configurations:Release"
        defines "RELEASE"
		optimize "On"
        symbols "On"
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
		"%{prj.name}/src/**.cpp",
	}

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
		links { "Shlwapi.lib", "%{prj.location}/vendor/benchmark/lib/Windows/%{cfg.buildcfg}/benchmark.lib" }
	filter "system:linux"
        links { "benchmark" }

project "game-engine"
	location "%{prj.name}"
	kind "ConsoleApp"

	targetdir("bin/" .. outputFolder .. "/%{prj.name}")
	objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	excludes { 
		"%{prj.name}/src/tetris/**.h", 
		"%{prj.name}/src/tetris/**.cpp"
	}

	links { "light-std" }
	includedirs { "light-std/src" }

	dependson { "tetris" }

	common_settings()
	
	filter "system:windows"
		links { "dxgi.lib", "d3d11.lib", "$(DXSDK_DIR)Lib/x64/d3dx11.lib", "$(DXSDK_DIR)Lib/x64/d3dx10.lib" }
		includedirs { "$(DXSDK_DIR)Include" }

project "tetris"
	location "game-engine"
	kind "SharedLib"

	targetdir("bin/" .. outputFolder .. "/game-engine")
	objdir("bin-int/" .. outputFolder .. "/game-engine")

	files {
		"game-engine/src/tetris/**.h", 
		"game-engine/src/tetris/**.cpp"
	}

	defines { "LE_BUILDING_GAME" }

	links { "light-std" }
	includedirs { "light-std/src", "game-engine/src" }

	common_settings()

	-- Unique PDB name each time we build (in order to support debugging while hot-swapping the game dll)
	symbolspath '$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString("ddMMyyyy_HHmmss_fff")).pdb'
