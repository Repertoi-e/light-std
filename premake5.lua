workspace "cpp-utils"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "cpp-utils"
    location "cpp-utils"
    kind "StaticLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/cppu/**.hpp",
        "%{prj.name}/src/cppu/**.cpp"
    }

    includedirs
    {
    
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines 
        {
            "CPPU_PLATFORM_WINDOWS",
            "_HAS_EXCEPTIONS=0"
        }

        excludes
        {
            "%{prj.name}/src/cppu/posix_*.cpp"
        }

    filter "configurations:Debug"
        defines "CPPU_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "CPPU_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "CPPU_DIST"
        optimize "On"


project "cpp-utils-tests"
    location "cpp-utils-tests"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "cpp-utils/src"
    }

    links 
    {
        "cpp-utils"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines 
        {
            "CPPU_PLATFORM_WINDOWS",
            "_HAS_EXCEPTIONS=0"
        }

    filter "configurations:Debug"
        defines "CPPU_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "CPPU_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "CPPU_DIST"
        optimize "On"

project "benchmark"
    location "benchmark"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "cpp-utils/src",
        "C:/Program Files/benchmark/include",
        "C:/Program Files/FMT/include"
    }

    links 
    {
        "cpp-utils",
        "C:/Program Files/benchmark/lib/benchmark.lib",
        "C:/Program Files/FMT/lib/fmt.lib"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines 
        {
            "CPPU_PLATFORM_WINDOWS",
            "_HAS_EXCEPTIONS=0"
        }

        links
        {
            "Shlwapi.lib"
        }
        
    filter "configurations:Debug"
        defines "CPPU_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "CPPU_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "CPPU_DIST"
        optimize "On"