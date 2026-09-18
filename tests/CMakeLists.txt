add_executable(test_brc test_brc.c)
target_link_libraries(test_brc PRIVATE Orbitra::orbitra)
target_compile_definitions(test_brc PRIVATE
    ORBITRA_VECTOR_FILE="${CMAKE_CURRENT_SOURCE_DIR}/vectors_v1.csv")
orbitra_enable_strict_warnings(test_brc)
add_test(NAME brc_validation COMMAND test_brc)

add_executable(consumer_smoke consumer_smoke.c)
target_link_libraries(consumer_smoke PRIVATE Orbitra::orbitra)
orbitra_enable_strict_warnings(consumer_smoke)
add_test(NAME consumer_smoke COMMAND consumer_smoke)

if(ORBITRA_HAVE_CXX)
    add_executable(cpp_header_smoke cpp_header_smoke.cpp)
    target_link_libraries(cpp_header_smoke PRIVATE Orbitra::orbitra)
    target_compile_features(cpp_header_smoke PRIVATE cxx_std_11)
    if(MSVC)
        target_compile_options(cpp_header_smoke PRIVATE /W4 /WX)
    else()
        target_compile_options(cpp_header_smoke PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Werror)
    endif()
    add_test(NAME cpp_header_smoke COMMAND cpp_header_smoke)
endif()

add_test(NAME installed_consumer
    COMMAND ${CMAKE_COMMAND}
        -DORBITRA_BUILD_DIR=${PROJECT_BINARY_DIR}
        -DORBITRA_TEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/install-consumer
        "-DORBITRA_C_FLAGS=${CMAKE_C_FLAGS}"
        -DORBITRA_CONFIG=$<CONFIG>
        -P ${CMAKE_CURRENT_SOURCE_DIR}/install_consumer.cmake)
