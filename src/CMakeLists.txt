target_sources(
    pooper-cube PRIVATE

    vulkan-instance.cpp
    vulkan-objects.hpp
    devices.cpp
    vulkan-debug.cpp
    window.hpp
    common.hpp
    window.cpp
    main.cpp
    pch.hpp
    swapchain.cpp
    commands.cpp
)

target_precompile_headers(pooper-cube PRIVATE pch.hpp)
