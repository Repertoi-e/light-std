workspace "light-std"
    architecture "x64"
    configurations { "Debug", "DebugOptimized", "Release" }
    location(_OPTIONS["to"] or "build/" .. _ACTION)

    filter { "action:gmake2" }
        toolset "clang"
    filter {}

    startproject "test-suite"

    -- Set this explicitly so LSTD knows which 
    -- OS to do. Otherwise we try to guess the platform
    -- based on compiler macros, which might not be 
    -- the best. For example, set this to NO_OS if you are
    -- programming for baremetal or something.
    --
    -- defines { "LSTD_NO_OS" }

-- These are to be tested
LSTD_INCLUDE_EXTRAS = { "guid", "signal" }

OUT_DIR = "build/bin/%{cfg.buildcfg}/%{prj.name}"
INT_DIR = "build/bin-int/%{cfg.buildcfg}/%{prj.name}"

-- By default we build without namespace, so everything is global.
-- Set for the name of the namespace (for example  LSTD_NAMESPACE = "lstd")
-- to encapsulate the whole library.  
-- LSTD_NAMESPACE = "lstd"

-- These options are optional and control how much memory the platform allocators
-- reserve upfront. Increase this if you get platform warnings with the message 
-- "Not enough memory  in the temporary/persistent allocator; expanding the pool".
-- Decrease this if you want to tweak the memory footprint of your application.
--
-- Note: Feel free to modify the library source code however you like. We just try 
-- to be as general as possible.
--
-- (KiB and MiB are literal operators that are defined in the library, 1_KiB = 1024, 1_MiB = 1024 * 1024)
--
-- @TODO: Have a clearer picture on memory usage. Persisent storage size can be calculated. 
--        Allow turning off certain options in order to make the persistent block smaller,
--        thus reducing the memory footprint of the library.
-- LSTD_PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE = "16_KiB"
-- LSTD_PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE = "1_MiB"

group "lstd"
    include "premake5-lstd"
group ""

group "test-suite"
    include "test-suite"
    include "example"
group ""
