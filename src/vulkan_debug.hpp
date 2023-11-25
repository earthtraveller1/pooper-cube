#pragma once

#include <vulkan/vulkan.h>

// Proxy functions for the Vulkan debugging capabilities.

namespace pooper_cube {
    extern const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO;

    class vulkan_debug_messenger_t {
        public:
            explicit vulkan_debug_messenger_t(VkInstance instance);

            struct creation_exception_t {
                VkResult result;
            };

            vulkan_debug_messenger_t(const vulkan_debug_messenger_t&) = delete;
            auto operator=(const vulkan_debug_messenger_t&) -> vulkan_debug_messenger_t& = delete;

            vulkan_debug_messenger_t(vulkan_debug_messenger_t&& source);
            auto operator=(vulkan_debug_messenger_t&& right_hand_side) -> vulkan_debug_messenger_t&;

            ~vulkan_debug_messenger_t() noexcept;
        private:
            VkDebugUtilsMessengerEXT m_handle;
            VkInstance m_instance;
    };
}
