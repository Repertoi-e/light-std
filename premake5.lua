newoption {
    trigger = "no-crt",
    description = "Disable linking with the Visual C++ Runtime Library on Windows."
}

newoption {
    trigger = "python",
    value = "path",
    description = "Path to Python 3.7 (e.g. C:/ProgramData/Anaconda3/ etc..., by default we try to find this automatically. We need the subdirectories include and lib)"
}

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

-- Vendor. We use mimalloc as our default malloc replacement. 
-- We modify the source code in order to be better suited for our uses.
project "mimalloc"
    location "%{prj.name}"
    kind "SharedLib"

    targetdir("bin/" .. outputFolder .. "/%{prj.name}")
    objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

	includedirs { "%{prj.name}/include" }
	
    files {
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/src/**.h"
	}
	
	excludes { "%{prj.name}/src/page-queue.c", "%{prj.name}/src/static.c", "%{prj.name}/src/alloc-override.c" }
	
	filter "system:windows"
		excludes "%{prj.name}/src/*-osx.c"
	filter {}
	
	defines { "MI_SHARED_LIB", "MI_SHARED_LIB_EXPORT", "MI_MALLOC_OVERRIDE" }
	
	links { "%{prj.location}/bin/mimalloc-redirect.lib" }
	
	-- Copy the x64 version to the target dir
	postbuildcommands {
		"{COPY} %{prj.location}/bin/mimalloc-redirect.dll %{cfg.targetdir}"
	}
    
    common_settings()

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

project "lstd-graphics"
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

    -- Exclude directx files on non-windows platforms since they would cause a compilation failure
    filter "not system:windows"
        excludes  { "%{prj.name}/src/d3d_*.h", "%{prj.name}/src/d3d_*.cpp" }

    filter {}

    links { "lstd" }
    includedirs { "lstd/src" }

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

project "benchmark"
    location "%{prj.name}"
    kind "ConsoleApp"

    targetdir("bin/" .. outputFolder .. "/%{prj.name}")
    objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    links { "lstd" }
    includedirs { "lstd/src", "%{prj.name}/vendor/benchmark/include" }
    
    common_settings()
    
    filter "system:windows"
        links { "shlwapi.lib", "%{prj.location}/vendor/benchmark/lib/Windows/%{cfg.buildcfg}/benchmark.lib" }
    filter "system:linux"
        links { "benchmark" }

project "game"
	location "%{prj.name}"
	location "%{prj.name}"
    kind "ConsoleApp"

    targetdir("bin/" .. outputFolder .. "/%{prj.name}")
    objdir("bin-int/" .. outputFolder .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    excludes { 
        "%{prj.name}/src/cars/**.h", 
        "%{prj.name}/src/cars/**.cpp",
        "%{prj.name}/src/physics/**.h", 
        "%{prj.name}/src/physics/**.cpp"
    }

    links { "lstd", "lstd-graphics" }
    includedirs { "lstd/src" }

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

    links { "lstd", "lstd-graphics" }
    includedirs { "lstd/src", "lstd-graphics/src", "game/src" }

    includedirs { "game/src" }
    pchheader "game.h"
    pchsource "game/src/game.cpp"
    forceincludes { "game.h" }
    
    common_settings()

    -- Unique PDB name each time we build (in order to support debugging while hot-swapping the game dll)
    filter "system:windows"
        symbolspath '$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString("ddMMyyyy_HHmmss_fff")).pdb'
        links { "dxgi.lib", "d3d11.lib", "d3dcompiler.lib", "d3d11.lib", "d3d10.lib" }


function query_terminal(command)
    local success, handle = pcall(io.popen, command)
    if not success then 
        return ""
    end

    result = handle:read("*a")
    handle:close()
    result = string.gsub(result, "\n$", "") -- Remove trailing whitespace
    return result
end

function get_python_path()
    local p = query_terminal('python -c "import sys; import os; print(os.path.dirname(sys.executable))"')
    p = string.gsub(p, "\\\\", "\\")
    p = string.gsub(p, "\\", "/")
    return p
end

function get_python_lib()
    return query_terminal("python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[-1]))[0]);\"")
end

function get_python_lib_from_option()
    py = path.getabsolute(_OPTIONS["python"])

    return query_terminal(py .. "/python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[-1]))[0]);\"")
end

py = get_python_path()
py_lib = get_python_lib()

if _OPTIONS["python"] then
    py = path.getabsolute(_OPTIONS["python"])
    py_lib = get_python_lib_from_option()
end

if (pythonPath == "" or pythonLib == "") and not _OPTIONS["python"] then
    error("Failed to find python! Please specify a path manually using the --python option.")
end

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

    links { "lstd", "lstd-graphics" }
    includedirs { "lstd/src", "lstd-graphics/src", "game/src" }

    excludes { "game/src/physics/python.cpp"}

    includedirs { py  .. "/include" }
    libdirs { py  .. "/libs" }
    links { py_lib }

    links { "lstd-python-graphics" }

    includedirs { "game/src" }
    pchheader "pch.h"
    pchsource "game/src/physics/pch.cpp"
    forceincludes { "pch.h" }

    dependson { "lstd-python-graphics" }

    common_settings()

    -- Unique PDB name each time we build (in order to support debugging while hot-swapping the game dll)
    filter "system:windows"
        symbolspath '$(OutDir)$(TargetName)-$([System.DateTime]::Now.ToString("ddMMyyyy_HHmmss_fff")).pdb'
        links { "dxgi.lib", "d3d11.lib", "d3dcompiler.lib", "d3d11.lib", "d3d10.lib" }

-- This is the python module used in physics
project "lstd-python-graphics"
    location "game"
    kind "SharedLib"
    
    targetdir("bin/" .. outputFolder .. "/game")
    objdir("bin-int/" .. outputFolder .. "/game/%{prj.name}")

    targetname("lstdgraphics")
    targetextension(".pyd")

    includedirs { py  .. "/include" }
    libdirs { py  .. "/libs" }
    links { py_lib }

    defines { "LE_BUILDING_GAME" }

    links { "lstd", "lstd-graphics" }
    includedirs { "lstd/src", "lstd-graphics/src", "game/src" }

    files {
        "game/src/physics/python.cpp"
    }

    common_settings()


project "bootloader"
    location "os/bootloader"
    kind "ConsoleApp"

    architecture "x64"
    language "C++"
    cppdialect "C++17"

    systemversion "latest"

    targetdir("bin/" .. outputFolder .. "/os")
    objdir("bin-int/" .. outputFolder .. "/os/%{prj.name}")

    targetname("x64_boot")
    targetextension(".efi")

    files {
        "os/%{prj.name}/src/**.h",
        "os/%{prj.name}/src/**.inc",
        "os/%{prj.name}/src/**.c",
        "os/%{prj.name}/src/**.cpp"
    }

    includedirs { "os/%{prj.name}/src/vendor/Uefi" }
    includedirs { "os/%{prj.name}/src/vendor/Uefi/X64" }

    defines "_HAS_EXCEPTIONS=0"
    exceptionhandling "Off"
	
    editandcontinue "Off"

    rtti "Off"
    characterset "Unicode"

    buildoptions { "/Gs9999999", "/utf-8" }
    linkoptions { "/SUBSYSTEM:EFI_APPLICATION", "/MANIFESTUAC:NO", "/DYNAMICBASE:NO", "/INCREMENTAL:NO", "/NXCOMPAT:NO", "/nodefaultlib", "/stack:\"0x100000\",\"0x100000\"" }

    defines { "BUILD_NO_CRT", "HAVE_USE_MS_ABI", "GNU_EFI_USE_EXTERNAL_STDARG" }

    flags { "NoRuntimeChecks", "NoBufferSecurityCheck" }
    flags { "OmitDefaultLibrary" }

    entrypoint "efi_main"

    filter "configurations:Debug"
        buildoptions { "BUILD_ONLY_IN_DIST" }
    filter "configurations:Release"
        buildoptions { "BUILD_ONLY_IN_DIST" }
    filter "configurations:Dist"
        defines "DIST"
        optimize "Full"
        symbols "Off"
     