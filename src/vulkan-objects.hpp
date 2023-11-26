#pragma once

#include <vulkan/vulkan.h>

namespace pooper_cube {
    struct instance_t {
        VkInstance handle;
        
        instance_t(bool p_enable_validation = false); 
        
        instance_t(const instance_t&) = delete;
        auto operator=(const instance_t&) -> instance_t& = delete;
        
        operator VkInstance() const noexcept {
            return handle;
        }
        
        ~instance_t() noexcept {
            vkDestroyInstance(handle, nullptr);
        }
    };

    class vulkan_debug_messenger_t {
        public:
            explicit vulkan_debug_messenger_t(VkInstance instance);

            vulkan_debug_messenger_t(const vulkan_debug_messenger_t&) = delete;
            auto operator=(const vulkan_debug_messenger_t&) -> vulkan_debug_messenger_t& = delete;

            vulkan_debug_messenger_t(vulkan_debug_messenger_t&& source) noexcept;
            auto operator=(vulkan_debug_messenger_t&& right_hand_side) noexcept -> vulkan_debug_messenger_t&;

            ~vulkan_debug_messenger_t() noexcept;
        private:
            VkDebugUtilsMessengerEXT m_handle;
            VkInstance m_instance;
    };

    struct no_adequate_physical_device_exception_t {};

    extern const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO;

    auto choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> VkPhysicalDevice; 
}
