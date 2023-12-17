add_custom_command(
    OUTPUT triangle.vert.spv
    COMMAND ${Vulkan_GLSLC_EXECUTABLE}
    ARGS -o ${CMAKE_CURRENT_SOURCE_DIR}/triangle.vert.spv ${CMAKE_CURRENT_SOURCE_DIR}/triangle.vert
    DEPENDS triangle.vert
)

add_custom_command(
    OUTPUT triangle.frag.spv
    COMMAND ${Vulkan_GLSLC_EXECUTABLE}
    ARGS -o ${CMAKE_CURRENT_SOURCE_DIR}/triangle.frag.spv ${CMAKE_CURRENT_SOURCE_DIR}/triangle.frag
    DEPENDS triangle.frag
)

add_custom_target(
    shaders DEPENDS

    triangle.vert.spv
    triangle.frag.spv
)

add_dependencies(pooper-cube shaders)
