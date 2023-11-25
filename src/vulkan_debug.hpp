#pragma once

#include <vulkan/vulkan.h>

// Proxy functions for the Vulkan debugging capabilities.

namespace pooper_cube {
    auto vk_create_debug_utils_messenger_ext(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* create_info,
        const VkAllocationCallbacks* allocator,
        VkDebugUtilsMessengerEXT* messenger
    ) -> VkResult;

    auto vk_destroy_debug_utils_messenger_ext(
        VkInstance instance,
        VkDebugUtilsMessengerEXT messenger,
        const VkAllocationCallbacks* allocator
    ) -> void;
}
