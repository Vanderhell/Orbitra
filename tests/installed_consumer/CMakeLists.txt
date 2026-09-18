cmake_minimum_required(VERSION 3.16)
project(OrbitraInstalledConsumer LANGUAGES C)

find_package(Orbitra 0.1 CONFIG REQUIRED)

add_executable(installed_consumer main.c)
target_link_libraries(installed_consumer PRIVATE Orbitra::orbitra)
enable_testing()
add_test(NAME installed_consumer COMMAND installed_consumer)
