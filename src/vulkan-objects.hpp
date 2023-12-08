#pragma once

#include "window.hpp"

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

    class debug_messenger_t {
        public:
            explicit debug_messenger_t(VkInstance instance);

            debug_messenger_t(const debug_messenger_t&) = delete;
            auto operator=(const debug_messenger_t&) -> debug_messenger_t& = delete;

            debug_messenger_t(debug_messenger_t&& source) noexcept;
            auto operator=(debug_messenger_t&& right_hand_side) noexcept -> debug_messenger_t&;

            ~debug_messenger_t() noexcept;
        private:
            VkDebugUtilsMessengerEXT m_handle;
            VkInstance m_instance;
    };

    struct physical_device_t {
        VkPhysicalDevice handle;
        uint32_t graphics_queue_family;
        uint32_t present_queue_family;

        operator VkPhysicalDevice() const noexcept { return handle; }
    };

    class device_t {
        public:
            explicit device_t(const physical_device_t& physical_device);

            device_t(const device_t&) = delete;
            auto operator=(const device_t&) = delete;

            operator VkDevice() const noexcept { return m_device; }

            auto get_graphics_queue() const noexcept { return m_graphics_queue; }

            auto get_present_queue() const noexcept { return m_present_queue; }

            ~device_t() noexcept { vkDestroyDevice(m_device, nullptr); }

        private:
            VkDevice m_device;
            VkQueue m_graphics_queue;
            VkQueue m_present_queue;
    };

    class swapchain_t {
        public:
            explicit swapchain_t(
                const window_t& window, 
                const physical_device_t& physical_device, 
                const device_t& device, 
                const window_t::surface_t& surface
            );

            swapchain_t(const swapchain_t&) = delete;
            auto operator=(const swapchain_t&) = delete;

            operator VkSwapchainKHR() const noexcept { return m_swapchain; }

            ~swapchain_t() noexcept {
                vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            }
            
        private:
            VkSwapchainKHR m_swapchain;

            std::vector<VkImage> m_images;
            std::vector<VkImageView> m_image_views;

            const device_t& m_device;
    };

    struct no_adequate_physical_device_exception_t {};

    extern const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO;

    auto choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> physical_device_t; 
}
