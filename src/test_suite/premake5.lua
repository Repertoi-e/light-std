project "test_suite"
    kind "ConsoleApp"
    architecture "x64"

    language "C++" 
    cppdialect "C++20"     

    characterset "Unicode"	

    targetdir("../../" .. OUT_DIR)
    objdir("../../" .. INT_DIR)

	files { "**.h", "**.inc", "**.c", "**.cpp", "**.def", "**.cppm" }
    
    link_lstd() -- This is defined in src/lstd/premake5.lua