#include "buffers.hpp"
#include "common.hpp"
#include "devices.hpp"

#include "images.hpp"

namespace pooper_cube {
    // Based on a code snippet from
    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    auto find_depth_format(const physical_device_t& p_physical_device) -> std::optional<VkFormat> {
        const std::array<VkFormat, 3> candidates {
            VK_FORMAT_D32_SFLOAT, 
            VK_FORMAT_D32_SFLOAT_S8_UINT, 
            VK_FORMAT_D24_UNORM_S8_UINT
        };
        
        for (auto candidate : candidates) {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(p_physical_device, candidate, &properties);

            if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                return candidate;
            }
        }

        return std::optional<VkFormat>{};
    }

    image_t::image_t(const physical_device_t& p_physical_device, const device_t& p_device, uint32_t p_width, uint32_t p_height, type_t p_type) : m_device(p_device) {
        VkImageCreateInfo image_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent = {
                .width = p_width,
                .height = p_height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        switch (p_type) {
            case type_t::sampled:
                image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
                image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                break;
            case type_t::depth_buffer:
                const auto format = find_depth_format(p_physical_device);
                if (!format.has_value()) {
                    throw generic_vulkan_exception_t{VK_SUCCESS, "There appears to be no usable depth format for some reasons."};
                }

                image_info.format = format.value();
                image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                break;
        }

        auto result = vkCreateImage(m_device, &image_info, nullptr, &m_image);
        if (result != VK_SUCCESS) {
            throw vulkan_creation_exception_t{result, "image"};
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(m_device, m_image, &memory_requirements);

        const auto memory_type_index = find_memory_type(p_physical_device, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (!memory_type_index.has_value()) {
            throw generic_vulkan_exception_t{VK_SUCCESS, "Failed to find an adequate memory type for the image."};
        }

        VkMemoryAllocateInfo allocate_info {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index.value(),
        };

        result = vkAllocateMemory(m_device, &allocate_info, nullptr, &m_memory);
        if (result != VK_SUCCESS) {
            throw vulkan_creation_exception_t{result, "memory for image"};
        }

        vkBindImageMemory(m_device, m_image, m_memory, 0);
    }

    image_t::~image_t() noexcept {
        vkFreeMemory(m_device, m_memory, nullptr);
        vkDestroyImageView(m_device, m_view, nullptr);
        vkDestroyImage(m_device, m_image, nullptr);
    }
}
