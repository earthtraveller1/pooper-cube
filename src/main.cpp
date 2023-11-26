#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>

#include <fmt/color.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "common.hpp"
#include "window.hpp"
#include "vulkan-objects.hpp"

auto main(int p_argc, char** p_argv) -> int {
    using pooper_cube::window_t;
    using pooper_cube::instance_t;
    using pooper_cube::vulkan_debug_messenger_t;
    using pooper_cube::vulkan_creation_exception_t;
    using pooper_cube::no_adequate_physical_device_exception_t;
    using pooper_cube::choose_physical_device;

    bool enable_validation = false;

    const std::vector<const char*> argv(p_argv, p_argv + p_argc);
    for (auto arg : argv) {
        if (std::strcmp(arg, "--enable-validation") == 0) {
            enable_validation = true;
        }
    }

    try {
        const window_t window{800, 600, "Pooper Cube"};
        const instance_t instance{enable_validation};
        std::optional<vulkan_debug_messenger_t> debug_messenger;
        const auto window_surface = window.create_vulkan_surface(instance);

        if (enable_validation) {
            debug_messenger = vulkan_debug_messenger_t{instance};
        }

        const auto physical_device = choose_physical_device(instance, window_surface);

        { 
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &device_properties);

            fmt::print(stderr, "[INFO]: Selected the {} graphics card.\n", device_properties.deviceName);
        }

        window.show();
        while (!window.should_close()) {
            window.poll_events();
        }
    } catch (window_t::creation_exception_t exception) {
        using exception_t = window_t::creation_exception_t;

        switch (exception) {
            case exception_t::glfw_init_failed:
                fmt::print(stderr, fmt::fg(fmt::color::red), ": Failed to initialize GLFW.\n");
                break;
            case exception_t::window_creation_failed:
                fmt::print(stderr, fmt::fg(fmt::color::red), "[FATAL ERROR]: Failed to create the GLFW window.\n");
                break;
        }

        return EXIT_FAILURE;
    } catch (vulkan_creation_exception_t& exception) {
        fmt::print(
            stderr, 
            fmt::fg(fmt::color::red), 
            "[FATAL ERROR]: Failed to create a Vulkan {}. Vulkan error {}.\n", 
            exception.object_name, 
            static_cast<int>(exception.error_code)
        );

        return EXIT_FAILURE;
    } catch (no_adequate_physical_device_exception_t& exception) {
        fmt::print(
            stderr,
            fmt::fg(fmt::color::red),
            "[FATAL ERROR]: Could not find an adequate physical device.\n"
        );

        return EXIT_FAILURE;
    }
}

