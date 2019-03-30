premake.tools.zapcc = {}

local local_zapcc = premake.tools.zapcc
local local_gcc = premake.tools.gcc

local_zapcc.getcflags             = local_gcc.getcflags
local_zapcc.getcxxflags           = local_gcc.getcxxflags
local_zapcc.getforceincludes      = local_gcc.getforceincludes
local_zapcc.getldflags            = local_gcc.getldflags
local_zapcc.getcppflags           = local_gcc.getcppflags
local_zapcc.getdefines            = local_gcc.getdefines
local_zapcc.getundefines          = local_gcc.getundefines
local_zapcc.getrunpathdirs        = local_gcc.getrunpathdirs
local_zapcc.getincludedirs        = local_gcc.getincludedirs
local_zapcc.getLibraryDirectories = local_gcc.getLibraryDirectories
local_zapcc.getlinks              = local_gcc.getlinks
local_zapcc.getmakesettings       = local_gcc.getmakesettings

function local_zapcc.gettoolname(cfg, tool)  
  if tool == "cc" then
    name = "zapcc"  
  elseif tool == "cxx" then
    name = "zapcc++"
  elseif tool == "ar" then
    name = "ar"
  else
    name = nil
  end
  return name
end

newoption {
    trigger     = "use-zapcc",
    description = "If you have zapcc installed, you can use it for debug builds to speed up building."
}

newoption {
    trigger     = "no-crt",
    description = "Disable linking with the Visual C++ Runtime Library on Windows."
}

workspace "light-std"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
function common_stuff()
    architecture "x64"

    rtti "Off"
    exceptionhandling "Off"
    floatingpointexceptions "off"
    defines "_HAS_EXCEPTIONS=0"

    filter "system:windows"
        defines "LSTD_PLATFORM_WINDOWS"
        excludes "%{prj.name}/src/posix_*.cpp"
    
    filter { "system:windows", "not options:no-crt" }
        staticruntime "on"
        excludes "%{prj.name}/src/windows_no_crt.cpp"

    filter { "system:windows", "options:no-crt" }
        defines "LSTD_NO_CRT"
        flags { "NoRuntimeChecks", "NoBufferSecurityCheck" }
        buildoptions { "/Gs9999999" }
    filter { "system:windows", "options:no-crt", "not kind:StaticLib" }
        linkoptions { "/nodefaultlib", "/subsystem:windows", "/stack:\"0x100000\",\"0x100000\"" }
        links { "Kernel32", "Shell32", "Winmm" }
        flags { "OmitDefaultLibrary" }
    filter { "system:windows", "options:no-crt", "kind:SharedLib" }
        entrypoint "main_no_crt_dll"
    filter { "system:windows", "options:no-crt", "kind:ConsoleApp" }
        entrypoint "main_no_crt"
  
    filter "system:linux"
        defines "LSTD_PLATFORM_LINUX"
        buildoptions "-fdiagnostics-absolute-paths"
        excludes "%{prj.name}/src/windows_*.cpp"
    filter { "system:linux", "options:use-zapcc" }
        toolset "zapcc"
    
    filter "system:macosx"
        defines "LSTD_PLATFORM_MAC"
        excludes "%{prj.name}/src/windows_*.cpp"
    filter { "system:macosx", "options:use-zapcc" }
        toolset "zapcc"

    filter "configurations:Debug"
        defines "LSTD_DEBUG"
        symbols "On"
    filter "configurations:Release"
        defines "LSTD_RELEASE"
        optimize "On"
    filter "configurations:Dist"
        defines "LSTD_DIST"
        optimize "On"
end

project "light-std"
    location "light-std"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {}

    common_stuff()
    

project "lstd-tests"
    location "lstd-tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "light-std/src"
    }

    links {
        "light-std"
    }

    common_stuff()

project "lstd-benchmark"
    location "lstd-benchmark"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "light-std/src"
    }

    links {
        "light-std"
    }

    defines {
        "LSTD_NAMESPACE_NAME=lstd"
    }

    common_stuff()
    filter "system:windows"
        includedirs {
            "C:/Program Files/benchmark/include",
            "C:/Program Files/FMT/include"
        }
        links {
            "C:/Program Files/benchmark/lib/benchmark.lib",
            "C:/Program Files/FMT/lib/fmt.lib",
            "Shlwapi.lib"
        }
    filter "system:linux"
        links {
            "benchmark", "fmt"
        }

project "lstd-engine"
    location "lstd-engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "light-std/src"
    }

    links {
        "light-std"
    }

    defines {
        "LE_BUILD_DLL"
    }

    -- Add projects that link to the engine here:
    postbuildcommands ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/math-grapher")

    common_stuff()

project "math-grapher"
    location "math-grapher"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "light-std/src",
        "lstd-engine/src",
    }

    links {
        "light-std",
        "lstd-engine"
    }

    common_stuff()