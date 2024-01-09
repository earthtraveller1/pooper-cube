#pragma once

#include "common.hpp"

namespace pooper_cube {
    class device_t;

    class image_t {
        public:
            image_t(const device_t& device, uint32_t width, uint32_t height);
            NO_COPY(image_t);

            ~image_t();

        private:
            VkImage m_image;
            VkImageView m_view;
            VkDeviceMemory m_memory;

            const device_t& m_device;
    };
}
