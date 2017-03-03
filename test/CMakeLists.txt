if (BuildTest)
  find_package(Boost COMPONENTS
    unit_test_framework
    REQUIRED
    )
  
  include_directories(.)
  aux_source_directory(. HyCu_SOURCES_TEST)

  add_executable(hycu-test
    ${HyCu_SOURCES_TEST}
    )
  target_link_libraries(hycu-test
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${FLINT_LIBRARY}
    ${GMP_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    hycu-test-lib
    )
  if (WithOpenCL)
    target_link_libraries(hycu-test ${OpenCL_LIBRARY})
  endif()

  add_custom_target(runtest ALL
    COMMAND hycu-test
    DEPENDS hycu-test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif (BuildTest)
