cmake_minimum_required(VERSION 3.0)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project (wargame)
if (NOT DEFINED CONAN_DEPENDENCIES)
  include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake OPTIONAL RESULT_VARIABLE HAS_CONAN)
  if (NOT HAS_CONAN)
    message(FATAL_ERROR "missing conanbuildinfo.cmake did you forget to run conan install ?")
  endif()
endif()
conan_basic_setup()

include_directories(.)

include(os)

set(SOURCES
  service.cpp
)

os_add_executable(wargame_service "Best wargame" ${SOURCES})
os_add_drivers(wargame_service virtionet)
os_add_stdout(wargame_service default_stdout)
os_add_plugins(wargame_service autoconf vfs)
os_diskbuilder(wargame_service drive)
