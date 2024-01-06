#pragma once

#include "common.hpp"
#include "devices.hpp"

namespace pooper_cube {
    class semaphore_t {
        public:
            semaphore_t(const device_t& device);
            NO_COPY(semaphore_t);

            operator VkSemaphore() const noexcept { return m_handle; }

            ~semaphore_t() noexcept {
                vkDestroySemaphore(m_device, m_handle, nullptr);
            }

        private:
            VkSemaphore m_handle;
            const device_t& m_device;
    };

    class fence_t {
        public:
            fence_t(const device_t& device);
            NO_COPY(fence_t);

            operator VkFence() const noexcept { return m_handle; }

            ~fence_t() noexcept {
                vkDestroyFence(m_device, m_handle, nullptr);
            }

        private:
            VkFence m_handle;
            const device_t& m_device;
    };
}
