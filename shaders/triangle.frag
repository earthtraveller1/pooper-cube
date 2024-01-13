#version 450

layout (location = 0) out vec4 out_color;

#include "triangle.glsl"

void main() {
    out_color = vec4(uniform_buffer.color_offset, 1.0, push_constants.color_offset, 1.0);
}
