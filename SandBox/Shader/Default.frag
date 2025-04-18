#version 450
/* This extension allows arrays of resources declared using unsized arrays to become run-time sized arrays. The 'require'
 * behavior causes the named extension to work, if the implementation does not support the extension, it fails
*/
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0)      in vec2 i_uv;
layout (location = 1)      in vec4 i_normal;
layout (location = 2)      in vec4 i_position;
layout (location = 3)      in vec4 i_view;
/* In general, there is not a 1:1 mapping between a vertex and a fragment. By default, the associated data per vertex is
 * interpolated across the primitive to generate the corresponding associated data per fragment. Using the `flat` keyword,
 * no interpolation is done, so every fragment generated during the rasterization of that particular primitive will get
 * the same data. Since primitives are usually defined by more than one vertex, this means that the data from only one
 * vertex is used in that case (called the provoking vertex)
*/
layout (location = 4) flat in uint i_diffuseTextureIdx;
layout (location = 5) flat in uint i_specularTextureIdx;
layout (location = 6) flat in uint i_emissionTextureIdx;
layout (location = 7) flat in uint i_shininess;
/* Unlike gl_Position in the vertex shader, there is no built-in variable to output a color for the current fragment. You
 * have to specify your own output variable for each frame buffer. The color is written to the variable that is linked to
 * the frame buffer at index specified in the layout modifier
*/
layout (location = 0) out vec4 o_color;

struct LightInstanceSBO {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
    float innerRadius;
    float outerRadius;
};

layout (set = 0, binding = 1) readonly buffer LightInstanceSBOContainer {
    LightInstanceSBO instances[];
} lightInstanceSBOContainer;

layout (set = 1, binding = 0) uniform sampler2D   textureSamplers[];
layout (set = 1, binding = 1) uniform samplerCube skyBoxSampler;

layout (push_constant) uniform LightTypeOffsetsPC {
layout (offset = 144)
    uint directionalLightsOffset;
    uint pointLightsOffset;
    uint spotLightsOffset;
    uint lightsCount;
} lightTypeOffsets;

vec3 createColor (const float reflectionIntensity) {
    /* We calculate a reflection vector around the object's normal vector based on the view direction, which is then used
     * as a direction vector to sample the cube map, returning a color value of the environment. The resulting effect is
     * that the object seems to reflect the sky box
     *
     *                                               Cube map top face
     *                                  +---------------------∆---------------------+
     *                                                        ^
     *                                                       /
     *                                   View direction     /Reflection
     *                                   \                 /
     *                                    \       Normal  /
     *                                     \      ^      /
     *                                      \     |     /
     *                                       \    |    /
     *                                        \   |   /
     *                                         \  |  /
     *                                          v | /
     *                      +---------------------∆---------------------+
     *                      |                                           |
     *                      +-------------------------------------------+
    */
    vec4 viewDirection = normalize (i_position - i_view);
    vec4 reflection    = reflect   (viewDirection, i_normal);

    return reflectionIntensity                            *
           vec3 (texture (skyBoxSampler, reflection.rgb)) *
           vec3 (texture (textureSamplers[i_specularTextureIdx], i_uv));

    /* Another form of environment mapping is refraction, where we use the refraction vector to sample from the cube map.
     * Refraction vector is fairly easy to implement using GLSL's built-in refract function that expects a normal vector,
     * a view direction, and a ratio between both materials' refractive indices. With the right combination of lighting,
     * reflection, refraction and vertex movement, we can create pretty neat water graphics
    */
}

vec3 createColor (const LightInstanceSBO lightInstance, const vec4 lightDirection) {
    vec3 ambient            = lightInstance.ambient *
                              vec3 (texture (textureSamplers[i_diffuseTextureIdx], i_uv));
    /* Diffuse lighting gives the object more brightness the closer its fragments are aligned to the light rays from a
     * light source
     *
     *                                   Light direction
     *                                   ^
     *                                    \       Normal
     *                                     \      ^
     *                                      \  ø  |
     *                                       \----|
     *                                        \   |
     *                                         \  |
     *                                          \ |
     *                      +---------------------∆---------------------+
     *                      |                                           |
     *                      +-------------------------------------------+
    */
    float diffuseIntensity  = max (dot (i_normal, lightDirection), 0.0);
    vec3 diffuse            = diffuseIntensity      *
                              lightInstance.diffuse *
                              vec3 (texture (textureSamplers[i_diffuseTextureIdx], i_uv));
    /* Similar to diffuse lighting, specular lighting is based on the light's direction and the object's normal vectors,
     * but this time it is also based on the view direction
     *
     *                                   Light direction   Reflection
     *                                   ^                 ^
     *                                    \       Normal  /
     *                                     \      ^      /           View direction
     *                                      \     |     /     ø      ^
     *                                       \    |    /------------/
     *                                        \   |   /        /
     *                                         \  |  /    /
     *                                          \ | //
     *                      +---------------------∆---------------------+
     *                      |                                           |
     *                      +-------------------------------------------+
     * With Phong lighting, we first calculate the dot product between the view direction and reflection vector and then
     * raise it to the power of shininess value of the highlight. The higher the shininess value of an object, the more it
     * properly reflects the light instead of scattering it all around and thus the smaller the highlight becomes
     *
     *                      vec4 reflection = reflect (-lightDirection, i_normal);
     *
     * Note that, Phong lighting is a great and very efficient approximation of lighting, but its specular reflections
     * break down when the angle between the view direciton and reflection vector goes over 90 degrees and the resulting
     * dot product becomes negative. The Blinn-Phong shading model is an extension to the Phong shading model, where
     * instead of relying on a reflection vector we're using a so called halfway direction that is a unit vector exactly
     * halfway between the view direction and the light direction. The closer this halfway direction aligns with the
     * surface's normal vector, the higher the specular contribution
     *
     *                                   Light direction   Halfway direction
     *                                   ^                 ^
     *                                    \       Normal  /
     *                                     \      ^      /           View direction
     *                                      \     |  ø  /            ^
     *                                       \    |----/            /
     *                                        \   |   /        /
     *                                         \  |  /    /
     *                                          \ | //
     *                      +---------------------∆---------------------+
     *                      |                                           |
     *                      +-------------------------------------------+
     * Now, whatever direction the viewer looks from, the angle between the halfway direction and the surface's normal
     * vector never exceeds 90 degrees (unless the light is far below the surface of course)
    */
    vec4 viewDirection      = normalize (i_view - i_position);
    vec4 halfwayDirection   = normalize (viewDirection + lightDirection);
    /* Another subtle difference between Phong and Blinn-Phong shading is that the angle between the halfway direction
     * and the surface's normal vector is often shorter than the angle between the view direction and reflection vector.
     * As a result, to get visuals similar to Phong shading the specular shininess exponent has to be set a bit higher.
     * A general rule of thumb is to set it between 2 and 4 times the Phong shininess exponent
    */
    float specularIntensity = pow (max (dot (i_normal, halfwayDirection), 0.0), i_shininess);
    vec3 specular           = specularIntensity      *
                              lightInstance.specular *
                              vec3 (texture (textureSamplers[i_specularTextureIdx], i_uv));

    float distance          = length (vec4 (lightInstance.position, 1.0) - i_position);
    float attenuation       = 1.0 /  (lightInstance.constant  +
                                      lightInstance.linear    *  distance +
                                      lightInstance.quadratic * (distance * distance));
    ambient                *= attenuation;
    diffuse                *= attenuation;
    specular               *= attenuation;
    /* To create the effect of a smoothly-edged spot light, we use the inner and outer radius values such that if a
     * fragment is between the inner and the outer cone defined by their radius, it should calculate an intensity value
     * between 0.0 and 1.0. Similarly, if the fragment is inside the inner cone its intensity is equal to 1.0 and 0.0 if
     * the fragment is outside the outer cone
     *
     * We can calculate such a value using the following equation
     *
     *                      I = (theta - outer cone radius) / epsilon
     *
     * Epsilon is the cosine difference between the inner and the outer cone radius. The resulting I value is then the
     * intensity of the spotlight at the current fragment (we're basically interpolating between the outer cosine and the
     * inner cosine based on the theta value)
     *
     * We now have an intensity value that is either negative when outside the spotlight, higher than 1.0 when inside
     * the inner cone, and somewhere in between around the edges. If we properly clamp the values (to make sure the
     * intensity values won't end up outside the [0, 1] range) we won't need an if-else in the fragment shader and we
     * can simply multiply the light components with the calculated intensity value
    */
    float epsilon           = lightInstance.innerRadius - lightInstance.outerRadius;
    float theta             = dot   (lightDirection, normalize (vec4 (-lightInstance.direction, 0.0)));
    float intensity         = clamp ((theta - lightInstance.outerRadius) / epsilon, 0.0, 1.0);

    return (ambient + diffuse + specular) * intensity;
}

void main (void) {
    /* Emission */
    vec3 fragColor                     = vec3 (texture (textureSamplers[i_emissionTextureIdx], i_uv));
    /* Environment mapping */
    fragColor                         += createColor (0.25);
    /* Directional lights */
    for (uint i = lightTypeOffsets.directionalLightsOffset; i < lightTypeOffsets.pointLightsOffset; i++) {
        LightInstanceSBO lightInstance = lightInstanceSBOContainer.instances[i];
        vec4 lightDirection            = normalize   (vec4 (-lightInstance.direction, 0.0));
        fragColor                     += createColor (lightInstance, lightDirection);
    }
    /* Point lights */
    for (uint i = lightTypeOffsets.pointLightsOffset; i < lightTypeOffsets.spotLightsOffset; i++) {
        LightInstanceSBO lightInstance = lightInstanceSBOContainer.instances[i];
        vec4 lightDirection            = normalize   (vec4 (lightInstance.position, 1.0) - i_position);
        fragColor                     += createColor (lightInstance, lightDirection);
    }
    /* Spot lights */
    for (uint i = lightTypeOffsets.spotLightsOffset; i < lightTypeOffsets.lightsCount; i++) {
        LightInstanceSBO lightInstance = lightInstanceSBOContainer.instances[i];
        vec4 lightDirection            = normalize   (vec4 (lightInstance.position, 1.0) - i_position);
        fragColor                     += createColor (lightInstance, lightDirection);
    }

    o_color = vec4 (fragColor, 1.0);
}