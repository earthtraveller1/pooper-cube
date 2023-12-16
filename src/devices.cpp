#include "common.hpp"
#include "vulkan-objects.hpp"
#include <vulkan/vulkan_core.h>

using pooper_cube::device_t;

device_t::device_t(const physical_device_t& p_physical_device) {
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    const float queue_priority = 1.0f;

    if (p_physical_device.graphics_queue_family == p_physical_device.present_queue_family) {
        queue_create_infos.push_back(
            VkDeviceQueueCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = p_physical_device.graphics_queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_priority,
            }
        );
    } else {
        VkDeviceQueueCreateInfo queue_create_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = p_physical_device.graphics_queue_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        queue_create_infos.push_back(queue_create_info);
        queue_create_info.queueFamilyIndex = p_physical_device.present_queue_family;
        queue_create_infos.push_back(queue_create_info);
    }

    const char* const enabled_extensions[1] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const VkPhysicalDeviceFeatures enabled_features{0};

    const VkDeviceCreateInfo device_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = enabled_extensions,
        .pEnabledFeatures = &enabled_features,
    };

    const auto result = vkCreateDevice(p_physical_device, &device_info, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "logical device"};
    }

    vkGetDeviceQueue(m_device, p_physical_device.graphics_queue_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, p_physical_device.present_queue_family, 0, &m_present_queue);
}

auto pooper_cube::choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> physical_device_t {
    uint32_t device_count;
    vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(p_instance, &device_count, physical_devices.data());

    // A better approach would be to rank the devices and choose the best one, but for
    // now, this approach (choosing the first one that fulfills our requirements) should
    // do the job.

    for (const auto physical_device : physical_devices) {
        // Normally, I would exclude CPU implementations of Vulkan, but now that I think about it
        // , there really isn't much point to doing so.

        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

        // Not sure if this is even possible, but I placed it here just in case.
        if (queue_family_count == 0) {
            continue;
        }

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        std::optional<uint32_t> graphics_family, present_family;

        uint32_t i = 0;
        for (const auto& queue_family : queue_families) {
            if ((queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                graphics_family = i;
            }

            VkBool32 supports_presentation;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, p_surface, &supports_presentation);
            if (supports_presentation == VK_TRUE) {
                present_family = i;
            }

            // We don't need to continue iterating anymore once we've found both
            // queue families.

            if (graphics_family.has_value() && present_family.has_value()) {
                break;
            }

            ++i;
        }

        if (!graphics_family.has_value() || !present_family.has_value()) {
            continue;
        }

        uint32_t device_extension_count;
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);

        // This probably isn't possible, but just in case.
        if (device_extension_count == 0) {
            continue;
        }

        std::vector<VkExtensionProperties> device_extensions(device_extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, device_extensions.data());

        bool has_swapchain_extension;
        for (const auto& extension : device_extensions) {
            if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                has_swapchain_extension = true;
                break;
            }
        }

        if (!has_swapchain_extension) {
            continue;
        }

        uint32_t surface_format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, p_surface, &surface_format_count, nullptr);

        if (surface_format_count == 0) {
            continue;
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, p_surface, &present_mode_count, nullptr);

        if (present_mode_count == 0) {
            continue;
        }

        return physical_device_t{physical_device, graphics_family.value(), present_family.value()};
    }

    throw no_adequate_physical_device_exception_t{};
}
