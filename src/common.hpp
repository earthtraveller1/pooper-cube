#pragma once

// Some stuff that everyone uses.

namespace pooper_cube {
    struct vulkan_creation_exception_t {
        VkResult error_code;
        std::string_view object_name;
    };
}

#define NO_COPY(classname)                            \
    classname(const classname&) = delete;             \
    classname& operator=(const classname&) = delete;
