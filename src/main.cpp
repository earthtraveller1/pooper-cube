#include <cstdio>
#include <cstdlib>
#include <vector>

#include <fmt/color.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "window.hpp"

namespace {
    struct instance_t {
        VkInstance handle;
        
        struct creation_exception_t {
            VkResult result;
        };
        
        instance_t() {
            const VkApplicationInfo application_info {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
            };

            const VkInstanceCreateInfo create_info {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
                .pApplicationInfo = &application_info,
            };

            const auto result = vkCreateInstance(
                &create_info,
                nullptr,
                &handle
            );
            
            if (result != VK_SUCCESS) {
                throw creation_exception_t{result};
            }
        }
        
        instance_t(const instance_t&) = delete;
        auto operator=(const instance_t&) -> instance_t& = delete;
        
        operator VkInstance() const noexcept {
            return handle;
        }
        
        ~instance_t() noexcept {
            vkDestroyInstance(handle, nullptr);
        }
    };
}

auto main() -> int {
    using pooper_cube::window_t;
    try {
        const window_t window{800, 600, "Pooper Cube"};
        const instance_t instance;
        
        {
            uint32_t device_count;
            vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
            
            std::vector<VkPhysicalDevice> devices(device_count);
            vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
            
            for (auto device : devices) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(device, &properties);
                
                fmt::print(stderr, "[INFO]: Found physical device {}!\n", properties.deviceName);
            }
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
    } catch (instance_t::creation_exception_t exception) {
        fmt::print(stderr, fmt::fg(fmt::color::red), "[FATAL ERROR]: Failed to create a Vulkan instance. Vulkan error {}.", exception.result);
        return EXIT_FAILURE;
    }
}
