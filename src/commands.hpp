#pragma once

#include "common.hpp"
#include "devices.hpp"

namespace pooper_cube {
    struct command_pool_t {
        public:
            explicit command_pool_t(const device_t& device, uint32_t queue_family_index);
            NO_COPY(command_pool_t);

            operator VkCommandPool() const noexcept { return m_pool; }

            auto allocate_command_buffer() const -> VkCommandBuffer;

            ~command_pool_t() noexcept {
                vkDestroyCommandPool(m_device, m_pool, nullptr);
            }

        private:
            VkCommandPool m_pool;
            const device_t& m_device;
    };

}
