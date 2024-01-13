#version 450

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform push_constants_t {
    float color_offset;
} push_constants;

layout (binding = 0) uniform uniform_buffer_t {
    float color_offset;
} uniform_buffer;

void main() {
    out_color = vec4(uniform_buffer.color_offset, 1.0, push_constants.color_offset, 1.0);
}
