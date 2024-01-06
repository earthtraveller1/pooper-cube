target_sources(
    pooper-cube PRIVATE

    pipelines.cpp
    vulkan-instance.cpp
    buffers.hpp
    vulkan-objects.hpp
    devices.cpp
    commands.hpp
    vulkan-debug.cpp
    vulkan-debug.hpp
    pipelines.hpp
    swapchain.hpp
    vulkan-instance.hpp
    sync-objects.cpp
    buffers.cpp
    window.hpp
    common.hpp
    window.cpp
    main.cpp
    pch.hpp
    swapchain.cpp
    sync-objects.hpp
    commands.cpp
    devices.hpp
)

target_precompile_headers(pooper-cube PRIVATE pch.hpp)
