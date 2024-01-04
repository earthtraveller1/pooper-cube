#include <algorithm>

#include "common.hpp"

#include "vulkan-objects.hpp"

using pooper_cube::swapchain_t;

swapchain_t::swapchain_t(const window_t& p_window, const physical_device_t& p_physical_device, const device_t& p_device, const window_t::surface_t& p_surface) : m_device(p_device) {
    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_physical_device, p_surface, &surface_capabilities);

    m_extent = surface_capabilities.currentExtent;
    // When surface_capabilities.currentExtent is set to this special value, it indicates
    // that the "indicating that the surface size will be determined by the extent of a 
    // swapchain targeting the surface."
    // Source: https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap34.html#vkGetPhysicalDeviceSurfaceCapabilitiesKHR
    const uint32_t special_value = 0xFFFFFFFF;

    // In such cases, we must set the swap extent to the size of the window.
    if (m_extent.width == special_value || m_extent.height == special_value) {
        const auto [framebuffer_width, framebuffer_height] = p_window.get_framebuffer_dimensions();

        m_extent.width = std::clamp(
            static_cast<uint32_t>(framebuffer_width), 
            surface_capabilities.maxImageExtent.width,
            surface_capabilities.minImageExtent.width
        );

        m_extent.height = std::clamp(
            static_cast<uint32_t>(framebuffer_height), 
            surface_capabilities.maxImageExtent.height,
            surface_capabilities.minImageExtent.height
        );
    }

    uint32_t surface_format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface, &surface_format_count, nullptr);

    std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface, &surface_format_count, surface_formats.data());

    VkSurfaceFormatKHR chosen_surface_format{surface_formats.at(0)};
    for (const auto& surface_format : surface_formats) {
        const bool has_desired_format = surface_format.format == VK_FORMAT_R8G8B8A8_SRGB;
        const bool has_desired_color_space = surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        if (has_desired_format && has_desired_color_space) {
            chosen_surface_format = surface_format;
        }
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface, &present_mode_count, nullptr);

    std::vector<VkPresentModeKHR> present_modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface, &present_mode_count, present_modes.data());

    // Only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available.
    // According to https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain,
    // that is
    VkPresentModeKHR chosen_present_mode{VK_PRESENT_MODE_FIFO_KHR};
    for (const auto& present_mode : present_modes) {
        // I prefer to use mailbox, mainly because it allows me to get rid of 
        // screen tearing while also maintaining a decent amount of latency.
        // You can read more here:
        // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap34.html#VkPresentModeKHR
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosen_present_mode = present_mode;
        }
    }

    VkSwapchainCreateInfoKHR swapchain_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = p_surface,
        .minImageCount = std::min(surface_capabilities.minImageCount + 1, surface_capabilities.maxImageCount),
        .imageFormat = chosen_surface_format.format,
        .imageColorSpace = chosen_surface_format.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

        // We will worry about these later in the code (not in real life
        // of course, this is not a TODO).
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,

        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = chosen_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    const uint32_t queue_families[] = {
        p_physical_device.graphics_queue_family,
        p_physical_device.present_queue_family,
    };

    // Both the present and graphics queue are going to have to access the swapchain
    // images. The present queue needs to present it, and the graphics queue needs
    // to render onto it.
    if (p_physical_device.graphics_queue_family != p_physical_device.present_queue_family) {
        swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queue_families;
    }

    const auto result = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "swapchain"};
    }

    uint32_t image_count;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);

    m_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_images.data());

    // Now, we can get into creating the image views.
    // Usually, the image views are another object entirely, but this time, since
    // it's so closely tied to the swap chain, I've decided to just bundle them
    // together.

    m_image_views.resize(image_count);

    std::transform(
        m_images.cbegin(), 
        m_images.cend(), 
        m_image_views.begin(), 
        [this, &chosen_surface_format](VkImage p_image) -> VkImageView {
            const VkImageViewCreateInfo view_info {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .image = p_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = chosen_surface_format.format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };

            VkImageView view;
            const auto result = vkCreateImageView(m_device, &view_info, nullptr, &view);
            if (result != VK_SUCCESS) {
                throw vulkan_creation_exception_t{result, "image view"};
            }

            return view;
        }
    );
}

