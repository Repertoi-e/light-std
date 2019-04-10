cmake_minimum_required(VERSION 3.0)

project(LSTD)

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	add_compile_definitions(_HAS_EXCEPTIONS=0)
	
	set(CompilerFlags CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
	foreach(CompilerFlag ${CompilerFlags})
	  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	  string(REPLACE "/GR" "/GR-" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
	
	set(LSTD_COMPILER_OPTIONS /EHa- /fp:except-)
	add_compile_options(${LSTD_COMPILER_OPTIONS})
endif()

add_subdirectory(light-std)
add_subdirectory(lstd-tests)
