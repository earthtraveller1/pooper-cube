#include "vulkan-objects.hpp"

auto pooper_cube::choose_physical_device(VkInstance p_instance, VkSurfaceKHR p_surface) -> VkPhysicalDevice {
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

        std::vector<VkExtensionProperties> device_extensions;
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


        return physical_device;
    }

    throw no_adequate_physical_device_exception_t{};
}
