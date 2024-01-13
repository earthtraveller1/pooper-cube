#pragma once

#include "common.hpp"
#include "devices.hpp"

namespace pooper_cube {
    class device_t;

    class descriptor_layout_t {
        public:
            descriptor_layout_t(const device_t& device, std::span<const VkDescriptorSetLayoutBinding> bindings);
            NO_COPY(descriptor_layout_t);

            operator VkDescriptorSetLayout() const noexcept { return m_layout; }

            ~descriptor_layout_t() noexcept {
                vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
            }

        private:
            VkDescriptorSetLayout m_layout;
            const device_t& m_device;
    };

    class descriptor_pool_t {
        public:
            descriptor_pool_t(const device_t& device, std::span<const VkDescriptorPoolSize> sizes, uint32_t max_sets);
            NO_COPY(descriptor_pool_t);

            operator VkDescriptorPool() const noexcept { return m_pool; }

            auto allocate_set(const descriptor_layout_t& layout) const -> VkDescriptorSet;

            ~descriptor_pool_t() noexcept {
                vkDestroyDescriptorPool(m_device, m_pool, nullptr);
            }

        private:
            VkDescriptorPool m_pool;
            const device_t& m_device;
    };
}
