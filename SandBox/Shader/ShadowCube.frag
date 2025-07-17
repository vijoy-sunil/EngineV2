#version 450

layout (location = 0)      in vec4  i_position;
layout (location = 1)      in vec3  i_lightPosition;
layout (location = 2) flat in float i_farPlane;

void main (void) {
    float depth  = length (i_position.xyz - i_lightPosition);
    gl_FragDepth = depth / i_farPlane;
}