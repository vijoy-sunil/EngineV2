#version 450

layout (location = 0) in  vec2 i_uv;
layout (location = 1) in  vec3 i_position;
layout (location = 2) in  uint i_diffuseTextureIdx;

layout (location = 0) out vec2 o_uv;
layout (location = 1) out uint o_diffuseTextureIdx;

struct MeshInstanceSBO {
    mat4 modelMatrix;
    mat4 normalMatrix;      /* Unused */
    uint textureIdxLUT[64];
};

layout (set = 0, binding = 0) readonly buffer MeshInstanceSBOContainer {
    MeshInstanceSBO instances[];
} meshInstanceSBOContainer;

layout (push_constant) uniform ActiveCameraPC {
    vec3 position;          /* Unused */
    mat4 viewMatrix;
    mat4 projectionMatrix;
} activeCamera;

uint decodeTextureIdx (const uint oldTextureIdx) {
    uint readIdx = oldTextureIdx / 4;
    uint offset  = oldTextureIdx % 4;
    uint mask    = 255 << offset * 8;

    uint packet  = meshInstanceSBOContainer.instances[gl_InstanceIndex].textureIdxLUT[readIdx];
    packet       = packet & mask;
    return packet >> offset * 8;
}

void main (void) {
    gl_Position         = activeCamera.projectionMatrix *
                          activeCamera.viewMatrix       *
                          meshInstanceSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                          vec4 (i_position, 1.0);

    o_uv                = i_uv;
    o_diffuseTextureIdx = decodeTextureIdx (i_diffuseTextureIdx);
}