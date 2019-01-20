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
    architecture "x64"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/cppu/**.hpp",
        "%{prj.name}/src/cppu/**.cpp"
    }

    includedirs {}

    cppdialect "C++17"
    systemversion "latest"
    defines {
        "_HAS_EXCEPTIONS=0"
    }

    filter "system:windows"
        defines {
            "CPPU_PLATFORM_WINDOWS"
        }
        excludes {
            "%{prj.name}/src/cppu/posix_*.cpp"
        }
    filter "system:linux"
        defines {
            "CPPU_PLATFORM_LINUX"
        }
        buildoptions { "-fdiagnostics-absolute-paths" }
        excludes {
            "%{prj.name}/src/cppu/windows_*.cpp"
        }
    filter "system:macosx"
        defines {
            "CPPU_PLATFORM_MAC"
        }
        excludes {
            "%{prj.name}/src/cppu/windows_*.cpp"
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
    architecture "x64"

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

    cppdialect "C++17"
    systemversion "latest"
    defines {
        "_HAS_EXCEPTIONS=0"
    }

    filter "system:windows"
        defines {
            "CPPU_PLATFORM_WINDOWS"
        }
    filter "system:linux"
        defines {
            "CPPU_PLATFORM_LINUX"
        }
        buildoptions { "-fdiagnostics-absolute-paths" }
    filter "system:macosx"
        defines {
            "CPPU_PLATFORM_MAC"
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
    architecture "x64"

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

    cppdialect "C++17"
    systemversion "latest"
    defines {
        "_HAS_EXCEPTIONS=0"
    }

    filter "system:windows"
        defines {
            "CPPU_PLATFORM_WINDOWS"
        }
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
        defines {
            "CPPU_PLATFORM_LINUX"
        }
        links {
            "benchmark", "fmt"
        }
        buildoptions { "-fdiagnostics-absolute-paths" }
    filter "system:macosx"
        defines {
            "CPPU_PLATFORM_MAC"
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