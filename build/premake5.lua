
workspace "light-std"
    architecture "x64"
    configurations { "Debug", "DebugOptimized", "Release" }
	location(_OPTIONS["to"] or _ACTION)

BASE_DIR = path.join(_WORKING_DIR, "..")

function common_settings()
    architecture "x64"

    language "C++" 
	cppdialect "C++20" 

    rtti "Off"
	justmycode "Off"
	editandcontinue "Off"
    exceptionhandling "Off" -- SEH still works
	
	-- We don't link with the runtime library but this makes certain warnings
	-- having to do with us replacing malloc/free disappear. 
	-- (Including certain CRT headers define those functions with 
	-- __declspec(dllimport) which don't match with our definitions).
	--
	-- As for why CRT headers are included? Ask Windows.h .....
	staticruntime "On"
    
	targetdir(BASE_DIR .. "bin/%{cfg.buildcfg}/%{prj.name}")
    objdir(BASE_DIR .. "bin/%{cfg.buildcfg}/%{prj.name}/int")
	
	characterset "Unicode"	
	
	files {
        BASE_DIR .. "src/%{prj.name}/**.h",
        BASE_DIR .. "src/%{prj.name}/**.inc",
        BASE_DIR .. "src/%{prj.name}/**.c",
        BASE_DIR .. "src/%{prj.name}/**.cpp",
        BASE_DIR .. "src/%{prj.name}/**.def",
        BASE_DIR .. "src/%{prj.name}/**.cppm"
    }
	
    -- Define this if you include headers from the normal standard library (STL).
    -- If this macro is not defined we provide our own implementations of certain things 
    -- that are normally defined in the STL and on which certain C++ features rely on.
    -- (e.g. placement new, initializer_lists, the compare header - required by the 
    -- spaceship operator)
	--
	-- Note that stuff has changed and we no longer guarantee the define below works 
	-- since it got too cumbersome (because of fucking windows header files) and we 
	-- build everything normally without the CRT anyway. Still it might make easier
	-- to deal with if ever, for some reason, CRT is unavoidable.
	-- 
    -- defines { "LSTD_DONT_DEFINE_STD" }
	
    -- Uncomment this to build the library without a namespace
    defines { "LSTD_NO_NAMESPACE" }
    
    -- Uncomment this to use a custom namespace name for the library
    -- defines { "LSTD_NAMESPACE=my_lstd" }
    
    includedirs { BASE_DIR .. "src" }
	
	filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }
        
        excludes { BASE_DIR .. "src/**/posix_*.cpp" }

        -- We need _CRT_SUPPRESS_RESTRICT for some reason
        defines { "LSTD_NO_CRT", "NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SUPPRESS_RESTRICT" } 
    
		flags { "OmitDefaultLibrary", "NoRuntimeChecks", "NoBufferSecurityCheck", "NoIncrementalLink" }
		buildoptions { "/Gs9999999" }        
    filter { "system:windows", "not kind:StaticLib" }
		linkoptions { "/nodefaultlib", "/subsystem:windows", "/stack:\"0x100000\",\"0x100000\"" }
        links { "kernel32", "shell32", "winmm", "ole32", "dwmapi", "dbghelp" }
        
    -- Setup entry point
    filter { "system:windows", "kind:SharedLib" }
	    entrypoint "main_no_crt_dll"
    filter { "system:windows", "kind:ConsoleApp or WindowedApp" }
	    entrypoint "main_no_crt"
 
    -- Setup configurations and optimization level
    filter "configurations:Debug"
        defines "DEBUG"
		
		-- Trips an assert if you try to access an element out of bounds.
		-- Works for arrays and strings in the library. I don't think we can check raw C arrays...
		defines "LSTD_ARRAY_BOUNDS_CHECK"
		
        symbols "On"
        buildoptions { "/FS" }
    filter "configurations:DebugOptimized"
        defines { "DEBUG", "DEBUG_OPTIMIZED" }
        
		defines "LSTD_ARRAY_BOUNDS_CHECK"
		
		optimize "On"
		
		 -- Otherwise MSVC generates internal undocumented intrinsics which we can't provide .. shame
		floatingpoint "Strict"

        symbols "On"
        
		buildoptions { "/FS" }
    filter "configurations:Release"
        defines { "RELEASE", "NDEBUG" } 

        optimize "Full"
		floatingpoint "Strict"

		symbols "Off"
    
end

project "lstd"
    kind "StaticLib"
	
    -- These options control how much memory the platform allocators reserve upfront.
    -- Increase this if you get platform warnings with the message "Not enough memory 
	-- in the temporary/persistent allocator; expanding the pool".
    -- Decrease this if you want to tweak the memory footprint of your application.
    -- Note: Feel free to modify the library source code however you like. We just try 
	-- to be as general as possible.
    --
    -- (KiB and MiB are literal operators that are defined in the library, 1_KiB = 1024, 1_MiB = 1024 * 1024)
    --
    -- @TODO: Have a clearer picture on memory usage. Persisent storage size can be calculated. 
    --        Allow turning off certain options in order to make the persistent block smaller,
    --        thus reducing the memory footprint of the library.
    defines { "PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE=16_KiB", "PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE=1_MiB" }
    
	files {
		BASE_DIR .. "src/lstd/lstd.natvis"
	}
	
    common_settings()

project "test_suite"
    kind "ConsoleApp"
	
    -- excludes "%{prj.name}/src/build_test_table.cpp"

    links { "lstd" }
    
    common_settings()

     