#version 450

layout (location = 0) in  vec3  i_position;

layout (location = 0) out vec4  o_position;
layout (location = 1) out vec3  o_lightPosition;
layout (location = 2) out float o_farPlane;

struct MeshInstanceLiteSBO {
    mat4 modelMatrix;
};

layout (set = 0, binding = 0) readonly buffer MeshInstanceLiteSBOContainer {
    MeshInstanceLiteSBO instances[];
} meshInstanceLiteSBOContainer;

layout (push_constant) uniform ActiveLightPC {
    vec3 position;
    float farPlane;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} activeLight;

void main (void) {
    gl_Position     = activeLight.projectionMatrix *
                      activeLight.viewMatrix       *
                      meshInstanceLiteSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                      vec4 (i_position, 1.0);
    gl_Position.x  *= -1.0;

    o_position      = meshInstanceLiteSBOContainer.instances[gl_InstanceIndex].modelMatrix *
                      vec4 (i_position, 1.0);
    o_lightPosition = activeLight.position;
    o_farPlane      = activeLight.farPlane;
}