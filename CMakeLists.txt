cmake_minimum_required(VERSION 3.0)

project(LSTD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

include(cotire)

if(MSVC)
	add_compile_definitions(_HAS_EXCEPTIONS=0)
	
	set(vars
	  CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
	  CMAKE_CXX_FLAGS
	  CMAKE_CXX_FLAGS_INIT
	  CMAKE_CXX_FLAGS_DEBUG_INIT
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE_INIT
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT
	  CMAKE_CXX_FLAGS_RELWITHDEBINFO)

	foreach(var ${vars})
	  string(REPLACE "/MD" "/MT" ${var} "${${var}}")
	endforeach()
	
	foreach(var ${vars})
	  string(REPLACE "/GR" "/GR-" ${var} "${${var}}")
	endforeach()

	set(LSTD_COMPILER_OPTIONS /EHa- /fp:except- /utf-8)
	add_compile_options(${LSTD_COMPILER_OPTIONS})
endif()

add_subdirectory(light-std)
add_subdirectory(lstd-tests)
add_subdirectory(lstd-benchmark)
