#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0)      in  vec2 i_uv;
layout (location = 1)      in  vec4 i_normal;
layout (location = 2)      in  vec4 i_position;
layout (location = 3) flat in  uint i_diffuseTextureIdx;
layout (location = 4) flat in  uint i_specularTextureIdx;
layout (location = 5) flat in  uint i_emissionTextureIdx;
layout (location = 6) flat in  uint i_shininess;

layout (location = 0)      out vec4 o_normal;
layout (location = 1)      out vec4 o_position;
layout (location = 2)      out vec4 o_color0;
layout (location = 3)      out vec4 o_color1;
layout (location = 4)      out vec4 o_color2;

layout (set = 1, binding = 0) uniform sampler2D defaultSamplers[];

const float MIN_SHININESS = 32.0;
const float MAX_SHININESS = 128.0;

void main (void) {
    o_normal     = i_normal;
    o_position   = i_position;
    o_color0     = texture (defaultSamplers[i_diffuseTextureIdx],  i_uv);
    o_color1     = texture (defaultSamplers[i_specularTextureIdx], i_uv);
    o_color2.rgb = texture (defaultSamplers[i_emissionTextureIdx], i_uv).rgb;
    /* Normalize shininess */
    o_color2.a   = (i_shininess - MIN_SHININESS) / (MAX_SHININESS - MIN_SHININESS);
}