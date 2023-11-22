cmake_minimum_required(VERSION 3.13)

include(GNUInstallDirs)
include(FetchContent)

project(pooper-cube CXX)

option(CMAKE_EXPORT_COMPILE_COMMANDS "Whether to generate compile_commands.json or not (Ninja only)." ON)

find_package(Vulkan)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)

FetchContent_MakeAvailable(glfw)

add_executable(pooper-cube)
target_link_libraries(pooper-cube PRIVATE Vulkan::Vulkan)

add_subdirectory(src)

install(
    TARGETS pooper-cube
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
