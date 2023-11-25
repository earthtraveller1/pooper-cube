#include "vulkan_debug.hpp"
#include <vulkan/vulkan_core.h>

auto pooper_cube::vk_create_debug_utils_messenger_ext(
    VkInstance p_instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* messenger
) -> VkResult {
    const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(p_instance, "vkCreateDebugUtilsMessengerEXT")
    );

    if (function == nullptr) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    return function(p_instance, p_create_info, p_allocator, messenger);
}

auto pooper_cube::vk_destroy_debug_utils_messenger_ext(
    VkInstance p_instance,
    VkDebugUtilsMessengerEXT p_messenger,
    const VkAllocationCallbacks* p_allocator
) -> void {
    const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(p_instance, "vkDestroyDebugUtilsMessengerEXT")
    );

    if (function != nullptr) {
        function(p_instance, p_messenger, p_allocator);
    }
}
