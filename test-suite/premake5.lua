project "test-suite"
    kind "ConsoleApp"
    architecture "x64"

    language "C++" 
    cppdialect "C++20"     

    characterset "Unicode"	

    targetdir("../" .. OUT_DIR)
    objdir("../" .. INT_DIR)

    includedirs { "../include" }

	files { "**.h", "**.inc", "**.c", "**.cpp", "**.def" }
    
    link_lstd() -- This is defined in src/lstd/premake5.lua