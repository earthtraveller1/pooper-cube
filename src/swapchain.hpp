#pragma once

#include "common.hpp"
#include "window.hpp"
#include "devices.hpp"

namespace pooper_cube {
    class render_pass_t;

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

            auto get_image_views() const -> const std::vector<VkImageView>& {
                return m_image_views;
            }

            auto get_extent() const noexcept -> VkExtent2D {
                return m_extent;
            }

            auto get_format() const noexcept -> VkFormat {
                return m_format;
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
            VkFormat m_format;

            const device_t& m_device;
    };

    class framebuffers_t {
        public:
            framebuffers_t(const device_t& device) : m_device(device), m_framebuffers{} {}

            framebuffers_t(const device_t& device, const swapchain_t& swapchain, const render_pass_t& render_pass);
            NO_COPY(framebuffers_t);

            auto operator=(framebuffers_t&& other) -> const framebuffers_t& {
                for (auto framebuffer : m_framebuffers) vkDestroyFramebuffer(m_device, framebuffer, nullptr);
                m_framebuffers = std::move(other.m_framebuffers);
                return *this;
            }

            auto get(uint32_t index) const noexcept -> VkFramebuffer {
                return m_framebuffers.at(index);
            }

            ~framebuffers_t() {
                for (const auto framebuffer : m_framebuffers) {
                    vkDestroyFramebuffer(m_device, framebuffer, nullptr);
                }
            }

        private:
            std::vector<VkFramebuffer> m_framebuffers;
            const device_t& m_device;
    };
}
