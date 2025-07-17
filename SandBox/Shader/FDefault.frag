#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0)      in  vec2 i_uv;
layout (location = 1) flat in  uint i_diffuseTextureIdx;
layout (location = 0)      out vec4 o_color;

layout (set = 1, binding = 0) uniform sampler2D defaultSamplers[];

void main (void) {
    o_color = texture (defaultSamplers[i_diffuseTextureIdx], i_uv);
}