#version 450

layout (location = 0) in vec3 a_position;

#include "triangle.glsl"

void main() {
    gl_Position = uniform_buffer.projection * uniform_buffer.view * push_constants.model * vec4(a_position, 1.0);
}
