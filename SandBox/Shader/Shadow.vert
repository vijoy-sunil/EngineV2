#version 450

layout (location = 0) in vec3 i_position;

struct MeshInstanceLiteSBO {
    mat4 modelMatrix;
};

layout (set = 0, binding = 0) readonly buffer MeshInstanceLiteSBOContainer {
    MeshInstanceLiteSBO instances[];
} meshInstanceLiteSBOContainer;

layout (push_constant) uniform ActiveLightPC {
    vec3 position;          /* Unused */
    float farPlane;         /* Unused */
    mat4 viewMatrix;
    mat4 projectionMatrix;
} activeLight;

void main (void) {
    gl_Position = activeLight.projectionMatrix *
                  activeLight.viewMatrix       *
                  meshInstanceLiteSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                  vec4 (i_position, 1.0);
}