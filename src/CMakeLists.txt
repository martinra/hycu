set(HyCu_SOURCES_CURVE
  block_iterator.cc
  curve.cc
  curve_iterator.cc
  fq_element_table.cc
  reduction_table.cc
  single_curve_fp.cc
  )
if (WITH_OPENCL)
  set(HyCu_SOURCES_CURVE
    ${HyCu_SOURCES_CURVE}
    opencl/interface.cc
    opencl/buffer_evaluation.cc
    opencl/kernel_evaluation.cc
    opencl/kernel_reduction.cc
    opencl/program.cc
    opencl/program_evaluation.cc
    opencl/program_reduction.cc
    )
endif()

set(HyCu_SOURCES_CONFIG
  config/config_node.cc
  )

set(HyCu_SOURCES_STORE
  store/curve_data.cc
  store/file_store.cc
  store/store.cc
  store/store_data.cc
  )

set(HyCu_SOURCES_THREADED
  threaded/thread.cc
  threaded/thread_pool.cc
  )

set(HyCu_SOURCES_STANDALONE_WORKER_POOL
  worker_pool/standalone.cc
  )

set(HyCu_SOURCES_MPI_WORKER_POOL
  worker_pool/mpi.cc
  worker_pool/mpi_worker.cc
  )


if (BUILD_SINGLE)
  add_executable(hycu-single
    executables/single.cc
    ${HyCu_SOURCES_CURVE}
    )
  target_link_libraries(hycu-single
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${FLINT_LIBRARY}
    ${GMP_LIBRARY}
    ${YAML-cpp_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  if (WITH_OPENCL)
    target_link_libraries(hycu-single
      ${OpenCL_LIBRARY}
      )
  endif()
  install(TARGETS hycu-single DESTINATION bin)
endif (BUILD_SINGLE)


if (BUILD_THREADED)
  add_executable(hycu-threaded
    executables/threaded.cc
    ${HyCu_SOURCES_CURVE}
    ${HyCu_SOURCES_CONFIG}
    ${HyCu_SOURCES_STORE}
    ${HyCu_SOURCES_THREADED}
    ${HyCu_SOURCES_STANDALONE_WORKER_POOL}
    )
  target_link_libraries(hycu-threaded
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${FLINT_LIBRARY}
    ${GMP_LIBRARY}
    ${YAML-cpp_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  if (WITH_OPENCL)
    target_link_libraries(hycu-threaded
      ${OpenCL_LIBRARY}
      )
  endif()
  install(TARGETS hycu-threaded DESTINATION bin)
endif (BUILD_THREADED)


if (BUILD_MERGER)
  add_executable(hycu-merger
    executables/merger.cc
    ${HyCu_SOURCES_CURVE}
    ${HyCu_SOURCES_STORE}
    )
  target_link_libraries(hycu-merger
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${FLINT_LIBRARY}
    ${GMP_LIBRARY}
    )
  if (WITH_OPENCL)
    target_link_libraries(hycu-merger
      ${OpenCL_LIBRARY}
      )
  endif()
  install(TARGETS hycu-merger DESTINATION bin)
endif (BUILD_MERGER)


if (BUILD_MPI)
  find_package(MPI REQUIRED)
  find_package(Boost COMPONENTS
    serialization mpi
    REQUIRED
    )

  include_directories(${MPI_CXX_INCLUDE_PATH})

  add_executable (hycu-mpi
    executables/mpi.cc
    ${HyCu_SOURCES_CURVE}
    ${HyCu_SOURCES_CONFIG}
    ${HyCu_SOURCES_STORE}
    ${HyCu_SOURCES_THREADED}
    ${HyCu_SOURCES_MPI_WORKER_POOL}
    )
  target_compile_options(hycu-mpi PUBLIC ${MPI_CXX_COMPILE_FLAGS})

  target_link_libraries(hycu-mpi
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_MPI_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${FLINT_LIBRARY}
    ${GMP_LIBRARY}
    ${MPI_CXX_LIBRARIES}
    ${YAML-cpp_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  if (WITH_OPENCL)
    target_link_libraries(hycu-mpi
      ${OpenCL_LIBRARY}
      )
  endif()
  install(TARGETS hycu-mpi DESTINATION bin)
endif (BUILD_MPI)


if (BUILD_TEST)
  add_library(hycu-test-lib STATIC
    ${HyCu_SOURCES_CURVE}
    ${HyCu_SOURCES_STORE}
    ${HyCu_SOURCES_THREADED}
    ${HyCu_SOURCES_STANDALONE_WORKER_POOL}
    )
endif (BUILD_TEST)
