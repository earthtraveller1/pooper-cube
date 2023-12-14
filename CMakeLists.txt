cmake_minimum_required(VERSION 3.13)

project(pooper-cube CXX)

include(GNUInstallDirs)
include(FetchContent)

option(CMAKE_EXPORT_COMPILE_COMMANDS "Whether to generate compile_commands.json or not (Ninja only)." ON)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)

FetchContent_MakeAvailable(glfw fmt glm)

add_executable(pooper-cube)
target_link_libraries(pooper-cube PRIVATE Vulkan::Vulkan glfw fmt glm)

add_subdirectory(src)
add_subdirectory(shaders)

install(
    TARGETS pooper-cube
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
