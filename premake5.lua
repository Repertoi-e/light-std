
-- We now always do!
-- newoption {
--     trigger = "no-crt",
--     description = "Disable linking with the Visual C++ Runtime Library on Windows."
-- }

workspace "light-std"
    architecture "x64"
    configurations { "Debug", "Release", "Dist" }

function common_settings()
    architecture "x64"

    language "C++"

    -- We can't specify C++20 but at least on Windows, our generate_projects.bat replaces language standard with stdcpplatest in the .vcxproj files
    cppdialect "C++17" 

    rtti "Off"
    characterset "Unicode"
    
    editandcontinue "Off"

    defines "_HAS_EXCEPTIONS=0"
    exceptionhandling "Off"
	
    includedirs { "%{prj.name}/src" }

	-- We need to link with the CRT with a DLL in order to redirect the malloc/free calls with ours.
	filter { "not options:no-crt" }
		staticruntime "Off"
		excludes "%{prj.name}/src/windows_no_crt.cpp"
		
    filter "system:windows"
        excludes "%{prj.name}/**/posix_*.cpp"
        systemversion "latest"
        buildoptions { "/utf-8" }
        
        defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_SUPPRESS_RESTRICT" }
        links { "dwmapi.lib", "dbghelp.lib" }

	-- Setup build flags if we are doing no-crt.
    filter { "system:windows", "options:no-crt" }
        defines "BUILD_NO_CRT"
        flags { "NoRuntimeChecks", "NoBufferSecurityCheck" }
        buildoptions { "/Gs9999999" }
	filter { "system:windows", "options:no-crt"}
	    flags { "OmitDefaultLibrary" }	
    filter { "system:windows", "options:no-crt", "not kind:StaticLib" }
        linkoptions { "/nodefaultlib", "/subsystem:windows", "/stack:\"0x100000\",\"0x100000\"" }
        links { "kernel32", "shell32", "winmm", "ole32" }
    filter { "system:windows", "options:no-crt", "kind:SharedLib" }
        entrypoint "main_no_crt_dll"
    filter { "system:windows", "options:no-crt", "kind:ConsoleApp or WindowedApp" }
        entrypoint "main_no_crt"

	-- Setup configurations and optimization level
    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"
        buildoptions { "/FS" }
    filter "configurations:Release"
        defines { "RELEASE", "NDEBUG"}
        optimize "On"
        symbols "On"
        buildoptions { "/FS" }
    filter "configurations:Dist"
        defines { "DIST", "NDEBUG" } 
        optimize "Full"
        floatingpoint "Fast"
    filter {}
end


outputFolder = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


project "lstd"
    location "%{prj.name}"
    kind "StaticLib"

    targetdir("bin/" .. outputFolder .. "/%{prj.name}")
    objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

    files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.inc",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.def",
		"%{prj.name}/src/**.ixx"
	}
	
    common_settings()


project "test-suite"
    location "%{prj.name}"
    kind "ConsoleApp"

    targetdir("bin/" .. outputFolder .. "/%{prj.name}")
    objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

    files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.inc",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.def",
		"%{prj.name}/src/**.ixx"
	}

	excludes "%{prj.name}/src/build_test_table.cpp"

    links { "lstd" }
    includedirs { "lstd/src" }
	
	pchheader "pch.h"
    pchsource "%{prj.name}/src/pch.cpp"
    forceincludes { "pch.h" }
    
    common_settings()

     