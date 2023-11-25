#include <cstdio>
#include <cstdlib>
#include <vector>

#include <fmt/color.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "window.hpp"

namespace {
    VKAPI_ATTR auto VKAPI_CALL debug_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           p_message_severity,
        VkDebugUtilsMessageTypeFlagsEXT                  p_message_type,
        const VkDebugUtilsMessengerCallbackDataEXT*      p_callback_data,
        void*                                            p_user_data
    ) -> VkBool32 {
        fmt::color text_color;
        std::string_view severity_text;

        switch (p_message_severity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                text_color = fmt::color::gray;
                severity_text = "INFO";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
                text_color = fmt::color::gray;
                severity_text = "VERBOSE";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                text_color = fmt::color::yellow;
                severity_text = "WARNING";
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                text_color = fmt::color::yellow;
                severity_text = "ERROR";
                break;
            default:
                text_color = fmt::color::white;
                severity_text = "";
        }

        std::string_view type_text;

        switch (p_message_type) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                type_text = "GENERAL";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: 
                type_text = "PEFORMANCE";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                type_text = "VALIDATION";
                break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
                type_text = "DEVICE ADDRESS BINDING";
                break;
        }

        fmt::print(stderr, fmt::fg(text_color), "[VULKAN {} {}]: {}", 
                type_text, severity_text, p_callback_data->pMessage);

        return VK_FALSE;
    }

    const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
        .pfnUserCallback = debug_messenger_callback,
        .pUserData = nullptr,
    };

    struct instance_t {
        VkInstance handle;
        
        struct creation_exception_t {
            VkResult result;
        };
        
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

            if (p_enable_validation) {
                enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
            }

            const VkInstanceCreateInfo create_info {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
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
        fmt::print(stderr, fmt::fg(fmt::color::red), "[FATAL ERROR]: Failed to create a Vulkan instance. Vulkan error {}.", static_cast<int>(exception.result));
        return EXIT_FAILURE;
    }
}
