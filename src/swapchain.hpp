#pragma once

#include "window.hpp"
#include "devices.hpp"

namespace pooper_cube {
    class swapchain_t {
        public:
            swapchain_t(const device_t& device) : 
                m_swapchain(VK_NULL_HANDLE),
                m_extent{},
                m_images{},
                m_image_views{},
                m_device(device)
            {}

            explicit swapchain_t(
                const window_t& window, 
                const physical_device_t& physical_device, 
                const device_t& device, 
                const window_t::surface_t& surface
            );

            swapchain_t(const swapchain_t&) = delete;
            auto operator=(const swapchain_t&) = delete;

            auto operator=(swapchain_t&&) -> swapchain_t&;

            operator VkSwapchainKHR() const noexcept { return m_swapchain; }

            auto get_image(uint32_t index) const -> VkImage {
                return m_images.at(index);
            }

            auto get_image_view(uint32_t index) const -> VkImageView {
                return m_image_views.at(index);
            }

            auto get_extent() const noexcept -> VkExtent2D {
                return m_extent;
            }

            ~swapchain_t() noexcept {
                std::for_each(m_image_views.cbegin(), m_image_views.cend(), [this](auto view) { vkDestroyImageView(m_device, view, nullptr); });
                vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
            }
            
        private:
            VkSwapchainKHR m_swapchain;

            std::vector<VkImage> m_images;
            std::vector<VkImageView> m_image_views;

            VkExtent2D m_extent;

            const device_t& m_device;
    };

}
