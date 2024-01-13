target_sources(
    pooper-cube PRIVATE

    buffers.cpp
    buffers.hpp
    commands.cpp
    commands.hpp
    common.hpp
    devices.cpp
    devices.hpp
    images.cpp
    images.hpp
    main.cpp
    pch.hpp
    pipelines.cpp
    pipelines.hpp
    swapchain.cpp
    swapchain.hpp
    sync-objects.cpp
    sync-objects.hpp
    vulkan-debug.cpp
    vulkan-debug.hpp
    vulkan-instance.cpp
    vulkan-instance.hpp
    vulkan-objects.hpp
    window.cpp
    window.hpp
)

target_precompile_headers(pooper-cube PRIVATE pch.hpp)
