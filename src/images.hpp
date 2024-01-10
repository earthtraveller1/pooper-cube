#pragma once

#include "common.hpp"

namespace pooper_cube {
    class device_t;
    class physical_device_t;

    class image_t {
        public:
            enum class type_t {
                sampled, depth_buffer
            };
        
            image_t(const physical_device_t& physical_device, const device_t& device, uint32_t width, uint32_t height, type_t type);
            NO_COPY(image_t);

            operator VkImage() const noexcept { return m_image; }

            ~image_t() noexcept;

        private:
            VkImage m_image;
            VkImageView m_view;
            VkDeviceMemory m_memory;

            const device_t& m_device;
    };
}
