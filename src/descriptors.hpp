#pragma once

#include "common.hpp"
#include "devices.hpp"

namespace pooper_cube {
    class device_t;

    class descriptor_layout_t {
        public:
            descriptor_layout_t(const device_t& device, std::span<const VkDescriptorSetLayoutBinding> bindings);
            NO_COPY(descriptor_layout_t);

            ~descriptor_layout_t() noexcept {
                vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
            }

        private:
            VkDescriptorSetLayout m_layout;
            const device_t& m_device;
    };
}
