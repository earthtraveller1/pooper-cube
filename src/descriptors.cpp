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
}
