#include "descriptors.hpp"

namespace pooper_cube {
    descriptor_layout_t::descriptor_layout_t(const device_t& p_device, std::span<const VkDescriptorSetLayoutBinding> p_bindings) :
        m_device(p_device)
    {
        const VkDescriptorSetLayoutCreateInfo layout_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(p_bindings.size()),
            .pBindings = p_bindings.data(),
        };

        const auto result = vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr, &m_layout);
        if (result != VK_SUCCESS) {
            throw vulkan_creation_exception_t{result, "descriptor set layout"};
        }
    }

    descriptor_pool_t::descriptor_pool_t(const device_t& p_device, std::span<const VkDescriptorPoolSize> p_sizes, uint32_t p_max_sets) :
        m_device(p_device)
    {
        const VkDescriptorPoolCreateInfo pool_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = p_max_sets,
            .poolSizeCount = static_cast<uint32_t>(p_sizes.size()),
            .pPoolSizes = p_sizes.data()
        };

        const auto result = vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_pool);
        if (result != VK_SUCCESS) {
            throw vulkan_creation_exception_t{result, "descriptor pool"};
        }
    }

    auto descriptor_pool_t::allocate_set(const descriptor_layout_t& p_layout) const -> VkDescriptorSet {
        const VkDescriptorSetLayout layout = p_layout;

        const VkDescriptorSetAllocateInfo allocate_info {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout,
        };

        VkDescriptorSet set;
        const auto result = vkAllocateDescriptorSets(m_device, &allocate_info, &set);
        if (result != VK_SUCCESS) {
            throw vulkan_creation_exception_t{result, "descriptor set"};
        }

        return set;
    }
}
