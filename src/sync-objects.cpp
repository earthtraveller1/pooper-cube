#include "vulkan-objects.hpp"

using pooper_cube::semaphore_t;

semaphore_t::semaphore_t(const device_t& p_device) : m_device(p_device) {
    const VkSemaphoreCreateInfo semaphore_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    const auto result = vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_handle);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "semaphore"};
    }
}
