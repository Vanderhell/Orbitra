cmake_minimum_required(VERSION 3.16)
project(Orbitra VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(ORBITRA_IS_TOP_LEVEL ON)
else()
    set(ORBITRA_IS_TOP_LEVEL OFF)
endif()

option(ORBITRA_BUILD_TESTS "Build Orbitra tests" ${ORBITRA_IS_TOP_LEVEL})
option(ORBITRA_BUILD_EXAMPLES "Build Orbitra examples" ${ORBITRA_IS_TOP_LEVEL})
option(ORBITRA_BUILD_BENCHMARKS "Build Orbitra benchmarks" OFF)

function(orbitra_enable_strict_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wstrict-prototypes
            -Werror)
    endif()
endfunction()

add_library(orbitra STATIC src/brc.c)
add_library(Orbitra::orbitra ALIAS orbitra)
set_target_properties(orbitra PROPERTIES EXPORT_NAME orbitra)
target_include_directories(orbitra PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(orbitra PUBLIC c_std_11)
set_target_properties(orbitra PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
orbitra_enable_strict_warnings(orbitra)

install(TARGETS orbitra
    EXPORT OrbitraTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/orbitra DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT OrbitraTargets
    FILE OrbitraTargets.cmake
    NAMESPACE Orbitra::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Orbitra)

configure_package_config_file(
    cmake/OrbitraConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OrbitraConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Orbitra)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/OrbitraConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/OrbitraConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/OrbitraConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Orbitra)

if(ORBITRA_BUILD_TESTS)
    include(CheckLanguage)
    check_language(CXX)
    if(CMAKE_CXX_COMPILER)
        enable_language(CXX)
        set(ORBITRA_HAVE_CXX ON)
    else()
        set(ORBITRA_HAVE_CXX OFF)
    endif()
    enable_testing()
    add_subdirectory(tests)
endif()

if(ORBITRA_BUILD_EXAMPLES)
    foreach(example basic_sequence full_cycle ram_block_order)
        add_executable(${example} examples/${example}.c)
        target_link_libraries(${example} PRIVATE orbitra)
        orbitra_enable_strict_warnings(${example})
    endforeach()
endif()

if(ORBITRA_BUILD_BENCHMARKS)
    add_executable(benchmark_brc benchmarks/benchmark_brc.c)
    target_link_libraries(benchmark_brc PRIVATE orbitra)
    orbitra_enable_strict_warnings(benchmark_brc)
endif()
