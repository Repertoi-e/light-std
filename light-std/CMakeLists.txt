if(WIN32)
	file(GLOB_RECURSE SOURCES "src/windows_*.cpp" "src/lstd/*.cpp" "src/vendor/*.cpp")
else()
	file(GLOB_RECURSE SOURCES "src/posix_*.cpp" "src/lstd/*.cpp" "src/vendor/*.cpp")
endif()
	
if(MSVC)
	add_compile_definitions(NOMINMAX)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

add_library(light-std ${SOURCES})

set_target_properties(light-std
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/light-std"
  LIBRARY_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/light-std"
  RUNTIME_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/light-std"
)

target_include_directories(light-std PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
