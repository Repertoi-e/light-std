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

workspace "cpp-utils"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
function common_stuff()
    defines { "_HAS_EXCEPTIONS=0" }
    filter "system:windows"
        defines "CPPU_PLATFORM_WINDOWS"
        excludes "%{prj.name}/src/cppu/posix_*.cpp"
    filter "system:linux"
        defines "CPPU_PLATFORM_LINUX"
        buildoptions "-fdiagnostics-absolute-paths"
        excludes "%{prj.name}/src/cppu/windows_*.cpp"
    filter "system:macosx"
        defines "CPPU_PLATFORM_MAC"
        excludes "%{prj.name}/src/cppu/windows_*.cpp"

    filter "configurations:Debug"
        defines "CPPU_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "CPPU_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "CPPU_DIST"
        optimize "On"
    configuration "use-zapcc"
        toolset "zapcc"
end

project "cpp-utils"
    location "cpp-utils"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    architecture "x64"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/cppu/**.hpp",
        "%{prj.name}/src/cppu/**.cpp"
    }

    includedirs {}

    common_stuff()
    

project "cpp-utils-tests"
    location "cpp-utils-tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    architecture "x64"
    systemversion "latest"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "cpp-utils/src"
    }

    links {
        "cpp-utils"
    }

    common_stuff()

project "benchmark"
    location "benchmark"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    architecture "x64"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "cpp-utils/src"
    }

    links {
        "cpp-utils"
    }

    defines {
        "CPPU_NAMESPACE_NAME=cppu"
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

project "math-grapher"
    location "math-grapher"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    architecture "x64"
    systemversion "latest"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "cpp-utils/src"
    }

    links {
        "cpp-utils"
    }

    common_stuff()