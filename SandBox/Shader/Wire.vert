#version 450

layout (location = 0) in  vec3 i_position;
layout (location = 0) out vec3 o_color;

struct MeshInstanceSBO {
    vec3 color;
    mat4 modelMatrix;
};

layout (set = 0, binding = 0) readonly buffer MeshInstanceSBOContainer {
    MeshInstanceSBO instances[];
} meshInstanceSBOContainer;

layout (push_constant) uniform ActiveCameraPC {
    vec3 position;          /* Unused */
    mat4 viewMatrix;
    mat4 projectionMatrix;
} activeCamera;

void main (void) {
    gl_Position = activeCamera.projectionMatrix *
                  activeCamera.viewMatrix       *
                  meshInstanceSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                  vec4 (i_position, 1.0);

    o_color     = meshInstanceSBOContainer.instances[gl_InstanceIndex].color;
}