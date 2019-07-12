file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable (lstd-benchmark ${SOURCES})

set_target_properties(lstd-benchmark
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lstd-benchmark"
  LIBRARY_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lstd-benchmark"
  RUNTIME_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lstd-benchmark"
)

include_directories("vendor/benchmark/include")

# We link with google-benchmark using prebuilt binaries.
if(MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_link_libraries (lstd-benchmark LINK_PUBLIC light-std 
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/benchmark/lib/Windows/Debug/benchmark.lib")
	endif()
	if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		target_link_libraries (lstd-benchmark LINK_PUBLIC light-std
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/benchmark/lib/Windows/RelWithDebInfo/benchmark.lib")
	endif()
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_libraries (lstd-benchmark LINK_PUBLIC light-std 
		"${CMAKE_CURRENT_SOURCE_DIR}/vendor/benchmark/lib/Windows/Release/benchmark.lib")
	endif()
	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	    target_link_libraries(lstd-benchmark LINK_PUBLIC Shlwapi)
	endif()
endif()
