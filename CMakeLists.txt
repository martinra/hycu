cmake_minimum_required(VERSION 3.6.2)
project(HyCu)
list(APPEND CMAKE_MODULE_PATH "${HyCu_SOURCE_DIR}/cmake")

option(BuildSingle "build single curve version" ON)
option(BuildTest "build tests" OFF)
option(BuildThreaded "build threaded version" ON)
option(BuildMPI "build MPI version" OFF)
option(BuildMerger "build MPI version" ON)

option(WithOpenCL "compile with OpenCL support" OFF)


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
