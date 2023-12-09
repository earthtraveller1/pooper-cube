target_sources(
    pooper-cube PRIVATE

    commands.cpp
    devices.cpp
    main.cpp
    swapchain.cpp
    vulkan-debug.cpp
    vulkan-objects.hpp
    vulkan-instance.cpp
    window.cpp
    window.hpp
)

target_precompile_headers(pooper-cube PRIVATE pch.hpp)
