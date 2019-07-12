file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(lstd-tests ${SOURCES})

set_target_properties(lstd-tests
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lstd-tests"
  LIBRARY_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lstd-tests"
  RUNTIME_OUTPUT_DIRECTORY "${LSTD_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lstd-tests"
)

target_link_libraries(lstd-tests LINK_PUBLIC light-std)
