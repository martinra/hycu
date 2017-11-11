cmake_minimum_required(VERSION 3.5.1)
project(HyCu)
list(APPEND CMAKE_MODULE_PATH "${HyCu_SOURCE_DIR}/cmake")

option(BUILD_SINGLE "build single curve version" OFF)
option(BUILD_TEST "build tests" OFF)
option(BUILD_THREADED "build threaded version" OFF)
option(BUILD_MPI "build MPI version" OFF)
option(BUILD_MERGER "build MPI version" OFF)

option(TIMING "compile with timing output" OFF)
option(WITH_OPENCL "compile with OpenCL support" OFF)


if (TIMING)
  add_definitions("-D TIMING")
endif()

if (WITH_OPENCL)
  add_definitions("-D WITH_OPENCL")
endif()


find_package (Boost COMPONENTS
  system filesystem program_options
  REQUIRED
  )

if (WithOpenCL)
  find_package (OpenCL REQUIRED)
  include_directories (${OpenCL_INCLUDE_DIRS})
endif ()

find_package (FLINT REQUIRED)
include_directories(${FLINT_INCLUDE_DIR})

find_package (GMP REQUIRED)
include_directories(${GMP_INCLUDE_DIR})

find_package(Threads)
if (CMAKE_USE_PTHREADS_INIT)
  list(APPEND CMAKE_C_FLAGS "-pthread")
endif()

find_package (YAML-cpp REQUIRED)
include_directories(${YAML-cpp_INCLUDE_DIR})


include_directories (src) 


add_subdirectory(src)
add_subdirectory(test)
