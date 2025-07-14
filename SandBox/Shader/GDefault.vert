#version 450

layout (location = 0) in  vec2 i_uv;
layout (location = 1) in  vec3 i_normal;
layout (location = 2) in  vec3 i_position;
layout (location = 3) in  uint i_diffuseTextureIdx;
layout (location = 4) in  uint i_specularTextureIdx;
layout (location = 5) in  uint i_emissionTextureIdx;
layout (location = 6) in  uint i_shininess;

layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec4 o_normal;
layout (location = 2) out vec4 o_position;
layout (location = 3) out uint o_diffuseTextureIdx;
layout (location = 4) out uint o_specularTextureIdx;
layout (location = 5) out uint o_emissionTextureIdx;
layout (location = 6) out uint o_shininess;

struct MeshInstanceSBO {
    mat4 modelMatrix;
    mat4 normalMatrix;
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
    gl_Position          = activeCamera.projectionMatrix *
                           activeCamera.viewMatrix       *
                           meshInstanceSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                           vec4 (i_position, 1.0);

    o_uv                 = i_uv;
    /* Note that, the lighting calculations in the fragment shader are all done in world space, so we need to transform
     * the normal vectors to world space coordinates. However, it's not as simple as multiplying it with a model matrix
     *
     * First of all, normal vectors are only direction vectors and do not represent a specific position in space. Second,
     * normal vectors do not have a homogeneous coordinate (the w component). This means that translations should not
     * have any effect on the normal vectors. So if we want to multiply the normal vectors with a model matrix we want to
     * remove the translation part of the matrix by taking the upper-left 3x3 matrix of the model matrix (note that we
     * could also set the w component of a normal vector to 0 and multiply with the 4x4 matrix)
     *
     * Third, if the model matrix would perform a non-uniform scale, the vertices would be changed in such a way that the
     * normal vector is not perpendicular to the surface anymore. Whenever we apply a non-uniform scale (note: a uniform
     * scale only changes the normal's magnitude, not its direction, which is easily fixed by normalizing it) the normal
     * vectors are not perpendicular to the corresponding surface anymore which distorts the lighting. The trick to fixing
     * this behavior is to use a different model matrix specifically tailored for normal vectors. This matrix is called
     * the normal matrix
    */
    o_normal             = normalize (meshInstanceSBOContainer.instances[gl_InstanceIndex].normalMatrix *
                           vec4 (i_normal, 0.0));
    /* Since we're going to do all the lighting calculations in world space, we want a vertex position for the fragment
     * that is in world space first. We can accomplish this by multiplying the vertex position with the model matrix only
     * (not the view and projection matrix) to transform it to world space coordinates, which can easily be accomplished
     * in the vertex shader. The 3 world positions of the triangle will be interpolated to form the fragment position in
     * world space
    */
    o_position           = meshInstanceSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                           vec4 (i_position, 1.0);
    o_diffuseTextureIdx  = decodeTextureIdx (i_diffuseTextureIdx);
    o_specularTextureIdx = decodeTextureIdx (i_specularTextureIdx);
    o_emissionTextureIdx = decodeTextureIdx (i_emissionTextureIdx);
    o_shininess          = i_shininess;
}