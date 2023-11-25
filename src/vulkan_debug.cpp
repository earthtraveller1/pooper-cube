
#include <fmt/format.h>
#include <fmt/color.h>
#include <vulkan/vulkan_core.h>

#include "vulkan_debug.hpp"

namespace {
    auto vk_create_debug_utils_messenger_ext(
        VkInstance p_instance,
        const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
        const VkAllocationCallbacks* p_allocator,
        VkDebugUtilsMessengerEXT* messenger
    ) noexcept -> VkResult {
        const auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(p_instance, "vkCreateDebugUtilsMessengerEXT")
        );

        if (function == nullptr) {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        return function(p_instance, p_create_info, p_allocator, messenger);
    }

    auto vk_destroy_debug_utils_messenger_ext(
        VkInstance p_instance,
        VkDebugUtilsMessengerEXT p_messenger,
        const VkAllocationCallbacks* p_allocator
    ) noexcept -> void {
        const auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(p_instance, "vkDestroyDebugUtilsMessengerEXT")
        );

        if (function != nullptr) {
            function(p_instance, p_messenger, p_allocator);
        }
    }

    VKAPI_ATTR auto VKAPI_CALL debug_messenger_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           p_message_severity,
        VkDebugUtilsMessageTypeFlagsEXT                  p_message_type,
        const VkDebugUtilsMessengerCallbackDataEXT*      p_callback_data,
        void*                                            p_user_data
    ) noexcept -> VkBool32 
    {
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

        fmt::print(stderr, fmt::fg(text_color), "[VULKAN {} {}]: {}\n", 
                type_text, severity_text, p_callback_data->pMessage);

        return VK_FALSE;
    }
}

const VkDebugUtilsMessengerCreateInfoEXT pooper_cube::DEBUG_MESSENGER_CREATE_INFO {
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

using pooper_cube::vulkan_debug_messenger_t;

vulkan_debug_messenger_t::vulkan_debug_messenger_t(VkInstance p_instance) : m_instance(p_instance) {
    const auto result = vk_create_debug_utils_messenger_ext(p_instance, &DEBUG_MESSENGER_CREATE_INFO, nullptr, &m_handle);
    if (result != VK_SUCCESS) {
        throw creation_exception_t{result};
    }
}

vulkan_debug_messenger_t::vulkan_debug_messenger_t(vulkan_debug_messenger_t&& source):
    m_handle(source.m_handle), 
    m_instance(source.m_instance) 
{
    source.m_handle = VK_NULL_HANDLE;
    source.m_instance = VK_NULL_HANDLE;
}

auto vulkan_debug_messenger_t::operator=(vulkan_debug_messenger_t&& right_hand_side) -> vulkan_debug_messenger_t& {
    vk_destroy_debug_utils_messenger_ext(m_instance, m_handle, nullptr);

    m_instance = right_hand_side.m_instance;
    m_handle = right_hand_side.m_handle;

    right_hand_side.m_instance = VK_NULL_HANDLE;
    right_hand_side.m_handle = VK_NULL_HANDLE;

    return *this;
}

vulkan_debug_messenger_t::~vulkan_debug_messenger_t() noexcept {
    vk_destroy_debug_utils_messenger_ext(m_instance, m_handle, nullptr);
}
