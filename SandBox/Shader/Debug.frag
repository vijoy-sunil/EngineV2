#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0)      in  vec2 i_uv;
layout (location = 1) flat in  uint i_textureIdx;
layout (location = 0)      out vec4 o_color;

layout (set = 0, binding = 0) uniform sampler2D debugSamplers[];

const float NEAR_PLANE = 0.01;
const float FAR_PLANE  = 100.0;

float getLinearizedDepth (const float depth) {
    return (2.0 * NEAR_PLANE) / (FAR_PLANE + NEAR_PLANE - depth * (FAR_PLANE - NEAR_PLANE));
}

void main (void) {
    vec3 data = texture (debugSamplers[i_textureIdx], i_uv).rgb;
    switch (i_textureIdx) {
        case 0:     /* Un-normalized data */
        case 1:     data = abs (normalize (data));
                    break;
        case 2:     /* Normalized data    */
        case 3:     break;
        default:    /* Non-linear depth   */
                    data = vec3 (getLinearizedDepth (data.r));
                    break;
    }
    o_color = vec4 (data, 1.0);
}