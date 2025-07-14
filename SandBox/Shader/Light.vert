#version 450

layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec4 o_cameraPosition;

layout (push_constant) uniform ActiveCameraPC {
    vec3 position;
    mat4 viewMatrix;        /* Unused */
    mat4 projectionMatrix;  /* Unused */
} activeCamera;

/*  UV layout in vulkan                     Normalized device coordinates
 *  (0, 0)              (1, 0)  top ^       (-1, -1)            (1, -1)
 *  +-------------------+                   +-------------------+
 *  |                   |                   |                   |
 *  |       (u, v)      |                   |       (0, 0)      |
 *  |                   |                   |                   |
 *  +-------------------+                   +-------------------+
 *  (0, 1)              (1, 1)  bottom v    (-1, 1)             (1, 1)
*/
vec2 uvs[6]       = vec2[] (vec2 ( 0.0, 1.0), vec2 (1.0,  1.0), vec2 ( 1.0,  0.0),
                            vec2 ( 0.0, 1.0), vec2 (1.0,  0.0), vec2 ( 0.0,  0.0));

vec2 positions[6] = vec2[] (vec2 (-1.0, 1.0), vec2 (1.0,  1.0), vec2 ( 1.0, -1.0),
                            vec2 (-1.0, 1.0), vec2 (1.0, -1.0), vec2 (-1.0, -1.0));

void main (void) {
    gl_Position      = vec4 (positions[gl_VertexIndex], 0.0, 1.0);

    o_uv             = uvs[gl_VertexIndex];
    o_cameraPosition = vec4 (activeCamera.position, 1.0);
}