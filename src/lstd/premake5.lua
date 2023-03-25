newoption {
    -- TODO:
    trigger = "lstd-windows-link-runtime-library",
    description = [[ 
        Force linking with the C/C++ runtime library on Windows.
        By default, we don't and we try our hardest to provide 
        functionality to external libraries that may require it.

        On Linux we can't not-link with glibc, because it's 
        coupled with the POSIX operating system calls library,
        although they really should be separate. 
                  ]]
}

function setup_configurations()
    filter "configurations:Debug"
        defines "DEBUG"
		
		-- Trips an assert if you try to access an element out of bounds.
		-- Works for arrays and strings in the library. I don't think we can check raw C arrays...
		defines "LSTD_ARRAY_BOUNDS_CHECK"
		
        symbols "On"

    filter "configurations:DebugOptimized"
        defines { "DEBUG", "DEBUG_OPTIMIZED" }
        
		defines "LSTD_ARRAY_BOUNDS_CHECK"
		
		optimize "On"
        symbols "On"

    filter { "configurations:DebugOptimized", "not lstd-windows-link-runtime-library" }
		-- Otherwise MSVC generates internal undocumented intrinsics which we can't provide .. shame
		floatingpoint "Strict"
    
    filter "configurations:Release"
        defines { "RELEASE", "NDEBUG" } 

        optimize "Full"
		floatingpoint "Strict"

		symbols "Off"
		
	filter {}
end

function link_lstd()
    filter { "kind:not StaticLib" }
        links { "lstd" }

    if LSTD_NAMESPACE then
        defines { "LSTD_NAMESPACE=" .. LSTD_NAMESPACE }
    elseif LSTD_NAMESPACE == "" then
        defines { "LSTD_NO_NAMESPACE" }
    end

    filter { "system:linux", "files:**.cpp or files:**.cppm"}
        buildoptions { "-fmodules-ts", "-MMD" }
	
	filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }

        -- We need _CRT_SUPPRESS_RESTRICT for some reason
        defines { "NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SUPPRESS_RESTRICT" } 
    
    filter { "system:windows", "not lstd-windows-link-runtime-library" }
        rtti "Off"
        justmycode "Off"
        editandcontinue "Off"

        -- Even if we don't link with the runtime library, the following 
        -- line makes certain warnings having to do with us replacing malloc/free disappear. 
	    -- (Certain CRT headers are included which define those functions with 
	    -- __declspec(dllimport) which don't match with our definitions).
	    --
	    -- As for why CRT headers are somehow always included? Ask Windows.h .....
	    staticruntime "On"

        defines { "LSTD_NO_CRT" }
		flags { "OmitDefaultLibrary", "NoRuntimeChecks", "NoBufferSecurityCheck", "NoIncrementalLink" }
		buildoptions { "/Gs9999999" }   

    filter { "system:windows", "kind:ConsoleApp or SharedLib", "not lstd-windows-link-runtime-library" }
		linkoptions { "/nodefaultlib", "/subsystem:windows", "/stack:\"0x100000\",\"0x100000\"" }
        links { "kernel32", "shell32", "winmm", "ole32", "dwmapi", "dbghelp" }
        
    -- Setup entry point
    filter { "system:windows", "kind:SharedLib", "not lstd-windows-link-runtime-library" }
	    entrypoint "main_no_crt_dll"
    filter { "system:windows", "kind:ConsoleApp or WindowedApp", "not lstd-windows-link-runtime-library" }
	    entrypoint "main_no_crt"
    filter {}

    setup_configurations()
end

function include_extra(str)
	files {
        "../lstd_extra/" .. str .. "/**.h",
        "../lstd_extra/" .. str .. "/**.inc",
        "../lstd_extra/" .. str .. "/**.c",
        "../lstd_extra/" .. str .. "/**.cpp",
        "../lstd_extra/" .. str .. "/**.def",
        "../lstd_extra/" .. str .. "/**.cppm"
    }
end

project "lstd"
    kind "StaticLib"
    architecture "x64"

    language "C++" 
    cppdialect "C++20"     
	
	characterset "Unicode"
    
    targetdir("../../" .. OUT_DIR)
    objdir("../../" .. INT_DIR)

    if LSTD_PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE then
        defines { "PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE=" .. LSTD_PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE }
    else 
        defines { "PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE=16_KiB" }
    end

    if LSTD_PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE then
        defines { "PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE=" .. LSTD_PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE }
    else 
        defines { "PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE=1_MiB" }
    end

    includedirs { "../" }

	files { "**.h", "**.inc", "**.c", "**.cpp", "**.def", "**.cppm", "lstd.natvis" }
	
    filter { "files:lstd.h" }
        compileas "HeaderUnit"
    
    filter { "files:**.cppm" }
        compileas "C++"

    filter { "system:linux" }
        removefiles { "third_party/cephes/**" }
    filter { "lstd-windows-link-runtime-library" }
        removefiles { "third_party/cephes/**" }

    filter { "system:windows", "not lstd-windows-link-runtime-library"}
        removefiles { "platform/posix/**" }

        -- These are x86-64 assembly and obj files since we don't support 
        -- other architectures at the moment.
        files {
            "platform/windows/no_crt/longjmp_setjmp.asm",
            "platform/windows/no_crt/chkstk.asm"
        }

    filter { "system:linux" }
        removefiles { "platform/windows/**" }
    filter {}

    if LSTD_INCLUDE_EXTRAS then
        for _, extra in ipairs(LSTD_INCLUDE_EXTRAS) do
            include_extra(extra)
        end
    end

    link_lstd() -- This is just to get the common options, not actually linking.
	


