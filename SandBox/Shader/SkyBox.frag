#version 450

layout (location = 0) in  vec3 i_uv;
layout (location = 0) out vec4 o_color;

layout (set = 1, binding = 0) uniform samplerCube skyBoxSampler;

void main (void) {
    o_color = texture (skyBoxSampler, i_uv);
}