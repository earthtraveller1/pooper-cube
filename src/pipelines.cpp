#include <fstream>

#include "vulkan-objects.hpp"

using pooper_cube::shader_module_t;

shader_module_t::shader_module_t(const device_t& p_device, type_t p_type, std::string_view p_code_path) : m_device(p_device) {
    std::ifstream file{p_code_path.data(), std::ios::ate | std::ios::binary};
    if (!file) {
        throw file_opening_exception_t{p_code_path};
    }

    const size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0).read(buffer.data(), file_size);

    file.close();

    const VkShaderModuleCreateInfo module_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = file_size,
        .pCode = reinterpret_cast<const uint32_t*>(file_size),
    };

    const auto result = vkCreateShaderModule(m_device, &module_info, nullptr, &m_module);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "shader module"};
    }
}
