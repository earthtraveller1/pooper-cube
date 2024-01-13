layout (push_constant) uniform push_constants_t {
    mat4 model;
    float color_offset;
} push_constants;

layout (binding = 0) uniform uniform_buffer_t {
    mat4 view;
    mat4 projection;
    float color_offset;
} uniform_buffer;
