#version 450

layout (location = 0) in  vec3 i_position;
layout (location = 0) out vec3 o_uv;

struct MeshInstanceUBO {
    mat4 modelMatrix;
};

layout (set = 0, binding = 0) uniform MeshInstanceUBOContainer {
    MeshInstanceUBO instance;
} meshInstanceUBOContainer;

layout (push_constant) uniform ActiveCameraPC {
    vec3 position;          /* Unused */
    mat4 viewMatrix;
    mat4 projectionMatrix;
} activeCamera;

void main (void) {
    /* We want the sky box to be centered around the camera so that no matter how far the camera moves, the sky box won't
     * get any closer, giving the impression the surrounding environment is extremely large. Multiplying with the view
     * matrix however transforms the sky box by rotating, scaling and translating them. We want to remove the translation
     * part of the view matrix so only rotation will affect the sky box's transformation. This can be done by converting
     * the view matrix to a 3x3 matrix (removing translation) and converting it back to a 4x4 matrix
    */
    mat4 viewMatrix = mat4 (mat3 (activeCamera.viewMatrix));
    vec4 position   = activeCamera.projectionMatrix *
                      viewMatrix                    *
                      meshInstanceUBOContainer.instance.modelMatrix *
                      vec4 (i_position, 1.0);
    /* Set the z component equal to its w component which will result in a depth that is always equal to 1.0 */
    gl_Position     = position.xyww;
    /* When a cube is centered at the origin, each of its position is also a direction vector from the origin. This vector
     * is exactly what we need to get the corresponding texture value at that specific position. For this reason we only
     * need to supply the fragment shader with position and not texture coordinates. The fragment shader then takes these
     * as input to sample a samplerCube
    */
    o_uv            = i_position;
    /* Transform cube map coordinates into vulkan coordinate space */
    o_uv.x         *= -1.0;
}