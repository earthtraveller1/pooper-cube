#include "vulkan-objects.hpp"

using pooper_cube::buffer_t;

namespace {
    using pooper_cube::physical_device_t;

    auto find_memory_type(const physical_device_t& p_physical_device, uint32_t p_type_filter, VkMemoryPropertyFlags properties) -> std::optional<uint32_t> {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(p_physical_device, &memory_properties);

        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
            const bool is_suitable = p_type_filter & (1 << i);
            const bool has_properties = memory_properties.memoryTypes[i].propertyFlags & properties;

            if (is_suitable && has_properties) {
                return i;
            }
        }

        return std::optional<uint32_t>{};
    }
}

buffer_t::buffer_t(const physical_device_t& p_physical_device, const device_t& p_device, type_t p_type, VkDeviceSize p_size) : m_device(p_device) {
    const VkBufferCreateInfo buffer_info {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = p_size,
        .usage = [p_type]() -> VkBufferUsageFlags{
            switch (p_type) {
                case type_t::vertex:
                    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                case type_t::element:
                    return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
                case type_t::staging:
                    return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            }
        }(),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    const auto result = vkCreateBuffer(p_device, &buffer_info, nullptr, &m_buffer);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "vertex buffer"};
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(p_device, m_buffer, &memory_requirements);

    const auto memory_type_index = find_memory_type(
            p_physical_device, 
            memory_requirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
}