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
#include "vulkan_debug.hpp"

namespace {
    struct instance_t {
        VkInstance handle;
        
        instance_t(bool p_enable_validation = false) {
            const VkApplicationInfo application_info {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = "Pooper Cube",
                .apiVersion = VK_API_VERSION_1_3,
            };

            uint32_t glfw_extension_count = 0;
            const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
            std::vector<const char*> enabled_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
            std::vector<const char*> enabled_layers;
            const void* next_pointer = nullptr;

            if (p_enable_validation) {
                enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
                next_pointer = &pooper_cube::DEBUG_MESSENGER_CREATE_INFO;
            }

            const VkInstanceCreateInfo create_info {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = next_pointer,
                .pApplicationInfo = &application_info,
                .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
                .ppEnabledLayerNames = enabled_layers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
                .ppEnabledExtensionNames = enabled_extensions.data(),
            };

            const auto result = vkCreateInstance(
                &create_info,
                nullptr,
                &handle
            );
            
            if (result != VK_SUCCESS) {
                throw pooper_cube::vulkan_creation_exception_t{result, "instance"};
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

    struct no_adequate_physical_device_exception_t {};

    auto choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> VkPhysicalDevice {
        uint32_t device_count;
        vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(p_instance, &device_count, physical_devices.data());

        // A better approach would be to rank the devices and choose the best one, but for
        // now, this approach (choosing the first one that fulfills our requirements) should
        // do the job.

        for (const auto physical_device : physical_devices) {
            // Normally, I would exclude CPU implementations of Vulkan, but now that I think about it
            // , there really isn't much point to doing so.

            uint32_t queue_family_count;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

            // Not sure if this is even possible, but I placed it here just in case.
            if (queue_family_count == 0) {
                continue;
            }

            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

            std::optional<uint32_t> graphics_family, present_family;

            uint32_t i = 0;
            for (const auto& queue_family : queue_families) {
                if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                    graphics_family = i;
                }

                VkBool32 supports_presentation;
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, p_surface, &supports_presentation);
                if (supports_presentation == VK_TRUE) {
                    present_family = i;
                }

                // We don't need to continue iterating anymore once we've found both
                // queue families.

                if (graphics_family.has_value() && present_family.has_value()) {
                    break;
                }

                ++i;
            }

            if (!graphics_family.has_value() || !present_family.has_value()) {
                continue;
            }

            uint32_t device_extension_count;
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);

            // This probably isn't possible, but just in case.
            if (device_extension_count == 0) {
                continue;
            }

            std::vector<VkExtensionProperties> device_extensions;
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.data());

            bool has_swapchain_extension;
            for (const auto& extension : device_extensions) {
                if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    has_swapchain_extension = true;
                    break;
                }
            }

            if (!has_swapchain_extension) {
                continue;
            }


            return physical_device;
        }

        throw no_adequate_physical_device_exception_t{};
    }
}

auto main(int p_argc, char** p_argv) -> int {
    bool enable_validation = false;

    const std::vector<const char*> argv(p_argv, p_argv + p_argc);
    for (auto arg : argv) {
        if (std::strcmp(arg, "--enable-validation") == 0) {
            enable_validation = true;
        }
    }

    using pooper_cube::window_t;
    using pooper_cube::vulkan_debug_messenger_t;
    using pooper_cube::vulkan_creation_exception_t;

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

