#include "commands.hpp"

using pooper_cube::command_pool_t;

command_pool_t::command_pool_t(const device_t& p_device, uint32_t p_queue_family_index) : m_device(p_device) {
    const VkCommandPoolCreateInfo pool_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = p_queue_family_index,
    };

    const auto result = vkCreateCommandPool(m_device, &pool_info, nullptr, &m_pool);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "command pool"};
    }
}

auto command_pool_t::allocate_command_buffer() const -> VkCommandBuffer {
    const VkCommandBufferAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = m_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer buffer;
    const auto result = vkAllocateCommandBuffers(m_device, &alloc_info, &buffer);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "command buffer"};
    }

    return buffer;
}
