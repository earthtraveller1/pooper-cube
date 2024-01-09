#include "devices.hpp"

#include "images.hpp"

namespace pooper_cube {
    image_t::image_t(const device_t& p_device, uint32_t p_width, uint32_t p_height) : m_device(p_device) {
        const VkImageCreateInfo image_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .extent = {
                .width = p_width,
                .height = p_height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        const auto result = vkCreateImage(m_device, &image_info, nullptr, &m_image);
        if (result != VK_SUCCESS) {
            throw vulkan_creation_exception_t{result, "image"};
        }
    }
}
