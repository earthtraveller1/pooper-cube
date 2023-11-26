#include "common.hpp"

#include "vulkan-objects.hpp"

using pooper_cube::instance_t;

instance_t::instance_t(bool p_enable_validation) {
    const VkApplicationInfo application_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Pooper Cube",
        .apiVersion = VK_API_VERSION_1_3,
    };

    uint32_t glfw_extension_count = 0;
    const auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    std::vector<const char*> enabled_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> enabled_layers;
    const void* next_pointer = nullptr;

    if (p_enable_validation) {
        enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        next_pointer = &pooper_cube::DEBUG_MESSENGER_CREATE_INFO;
    }

    const VkInstanceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = next_pointer,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
        .ppEnabledLayerNames = enabled_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data(),
    };

    const auto result = vkCreateInstance(
        &create_info,
        nullptr,
        &handle
    );
    
    if (result != VK_SUCCESS) {
        throw pooper_cube::vulkan_creation_exception_t{result, "instance"};
    }
}
