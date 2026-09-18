if(NOT DEFINED ORBITRA_BUILD_DIR OR NOT DEFINED ORBITRA_TEST_ROOT)
    message(FATAL_ERROR "ORBITRA_BUILD_DIR and ORBITRA_TEST_ROOT are required")
endif()

file(REMOVE_RECURSE "${ORBITRA_TEST_ROOT}")
set(prefix "${ORBITRA_TEST_ROOT}/prefix")
set(consumer_build "${ORBITRA_TEST_ROOT}/build")
set(install_config_args)
set(build_config_args)
set(test_config_args)
set(consumer_build_type_args)
if(ORBITRA_CONFIG)
    list(APPEND install_config_args --config "${ORBITRA_CONFIG}")
    list(APPEND build_config_args --config "${ORBITRA_CONFIG}")
    list(APPEND test_config_args -C "${ORBITRA_CONFIG}")
    list(APPEND consumer_build_type_args "-DCMAKE_BUILD_TYPE=${ORBITRA_CONFIG}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --install "${ORBITRA_BUILD_DIR}"
        --prefix "${prefix}" ${install_config_args}
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Orbitra install failed: ${result}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${CMAKE_CURRENT_LIST_DIR}/installed_consumer"
        -B "${consumer_build}" "-DCMAKE_PREFIX_PATH=${prefix}"
        "-DCMAKE_C_FLAGS=${ORBITRA_C_FLAGS}"
        ${consumer_build_type_args}
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Installed consumer configure failed: ${result}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}" ${build_config_args}
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Installed consumer build failed: ${result}")
endif()

execute_process(
    COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${consumer_build}"
        ${test_config_args} --output-on-failure
    RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Installed consumer test failed: ${result}")
endif()
