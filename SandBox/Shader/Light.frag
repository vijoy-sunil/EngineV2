#version 450
#extension GL_EXT_nonuniform_qualifier: require

layout (location = 0) in  vec2 i_uv;
layout (location = 1) in  vec4 i_cameraPosition;
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
    float farPlane;

    mat4 viewMatrix;
    mat4 projectionMatrix;
};

layout (set = 0, binding = 0) readonly buffer LightInstanceSBOContainer {
    LightInstanceSBO instances[];
} lightInstanceSBOContainer;

layout (set = 1, binding = 0) uniform sampler2D   shadowSamplers[];
layout (set = 1, binding = 1) uniform samplerCube shadowCubeSamplers[];
layout (set = 1, binding = 2) uniform sampler2D   normalSampler;
layout (set = 1, binding = 3) uniform sampler2D   positionSampler;
layout (set = 1, binding = 4) uniform sampler2D   color0Sampler;        /* Diffuse                     */
layout (set = 1, binding = 5) uniform sampler2D   color1Sampler;        /* Specular                    */
layout (set = 1, binding = 6) uniform sampler2D   color2Sampler;        /* RGB: Emission, A: Shininess */
layout (set = 1, binding = 7) uniform samplerCube skyBoxSampler;

layout (push_constant) uniform LightTypeOffsetsPC {
layout (offset = 144)
    uint spotLightsOffset;
    uint pointLightsOffset;
    uint lightsCount;
} lightTypeOffsets;

const float REFLECTION_INTENSITY = 0.05;
const float MIN_SHININESS        = 32.0;
const float MAX_SHININESS        = 128.0;
const float MIN_DEPTH_BIAS       = 0.01;
const float MAX_DEPTH_BIAS       = 0.2;

float computeShadowFactor (const uint samplerIdx,
                           const vec4 positionLS,
                           const bool shadowOutsideFrustum) {
    /* The first thing to do to check whether a fragment is in shadow, is transform the light space fragment position in
     * clip space to normalized device coordinates. When we output a clip space vertex position to gl_Position in the
     * vertex shader, it automatically does a perspective divide which transforms clip space coordinates in the range
     * [(-w, -w, 0) to (w, w, w)] to [(-1, -1, 0) to (1, 1, 1)] by dividing the x, y and z component by the vector's w
     * component. As the clip space fragment position is not passed to the fragment shader through gl_Position, we have to
     * do this perspective divide ourselves
     *
     * Note that, when using an orthographic projection matrix the w component of a vertex remains untouched so this step
     * is actually quite meaningless. However, it is necessary when using perspective projection so keeping this line
     * ensures it works with both projection matrices
    */
    vec3 positionNDC = positionLS.xyz / positionLS.w;
    /* Note that, we are overriding the wrapping option specified in the sampler address mode by manually specifying what
     * happens when we sample outside the shadow image's [0, 1] coordinate range. Additionally, we specify what happens
     * when we sample a fragment depth outside the far plane of the light's projection frustum
    */
    if (abs (positionNDC.x) > 1.0 ||
        abs (positionNDC.y) > 1.0 ||
        abs (positionNDC.z) > 1.0)
        return shadowOutsideFrustum ? 0.0: 1.0;

    /* Transform the NDC coordinates in [-1, 1] to [0, 1] range to sample from the shadow image, z is already in [0, 1] */
    vec2 sampleUV       = positionNDC.xy * 0.5 + 0.5;
    float sampledDepth  = texture (shadowSamplers[samplerIdx], sampleUV).r;
    float fragmentDepth = positionNDC.z;

    return fragmentDepth > sampledDepth ? 0.0: 1.0;
    /* Shadow anti aliasing and percentage close filtering (PCF)
     * Because the shadow image has a fixed resolution, the depth usually spans more than one fragment per texel. As a
     * result, multiple fragments sample the same depth value and come to the same shadow conclusions, which produces
     * jagged blocky edges. The idea behind PCF is to sample more than once from the shadow image, each time with slightly
     * different texture coordinates. For each individual sample we check whether it is in shadow or not. All of the sub
     * results are then combined and averaged to get a nice soft looking shadow
    */
}

float computeShadowFactor (const LightInstanceSBO lightInstance,
                           const vec4 lightDirection,
                           const vec4 normal,
                           const vec4 position,
                           const uint samplerIdx,
                           const float minDepthBias,
                           const float maxDepthBias) {

    vec3 sampleDirection = position.xyz - lightInstance.position;
    float sampledDepth   = texture (shadowCubeSamplers[samplerIdx], sampleDirection).r * lightInstance.farPlane;
    float fragmentDepth  = length  (sampleDirection);
    /* A constant shadow bias value may solve the shadow acne to some extent, but you can imagine the bias value is highly
     * dependent on the angle between the light source and the surface. If the surface would have a steep angle to the
     * light source, the shadows may still display shadow acne. A more solid approach would be to change the amount of
     * bias based on the surface angle towards the light. This way, surfaces that are almost perpendicular to the light
     * source get a small bias, while others get a much larger bias
    */
    float bias = max (maxDepthBias * (1.0 - dot (normal, lightDirection)), minDepthBias);

    return fragmentDepth > (sampledDepth + bias) ? 0.0: 1.0;
}

vec3 computeColor (const vec4 normal,
                   const vec4 position,
                   const vec3 specularColor,
                   const float reflectionIntensity) {
    /* We calculate a reflection vector around the object's normal vector based on the view direction, which is then used
     * as a direction vector to sample the cube map, returning a color value of the environment. The resulting effect is
     * that the object seems to reflect the sky box
     *
     *                                               Cube map top face
     *                                  +---------------------∆---------------------+
     *                                                        ^
     *                                                       /
     *                                   View direction    Reflection
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
    vec4 viewDirection   = normalize (position - i_cameraPosition);
    vec4 reflection      = reflect   (viewDirection, normal);
    reflection.x        *= -1.0;
    vec3 reflectionColor = texture   (skyBoxSampler, reflection.rgb).rgb;

    return reflectionIntensity * reflectionColor * specularColor;
}

vec3 computeColor (const LightInstanceSBO lightInstance,
                   const vec4 lightDirection,
                   const vec4 normal,
                   const vec4 position,
                   const vec3 diffuseColor,
                   const vec3 specularColor,
                   const float shininess,
                   const float shadowFactor) {

    vec3 ambientFactor = lightInstance.ambient * diffuseColor;
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
    float diffuseIntensity = max (dot (normal, lightDirection), 0.0);
    vec3 diffuseFactor     = diffuseIntensity * lightInstance.diffuse * diffuseColor;
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
     *                      vec4 reflection = reflect (-lightDirection, normal);
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
    vec4 viewDirection    = normalize (i_cameraPosition - position);
    vec4 halfwayDirection = normalize (viewDirection + lightDirection);
    /* Another subtle difference between Phong and Blinn-Phong shading is that the angle between the halfway direction
     * and the surface's normal vector is often shorter than the angle between the view direction and reflection vector.
     * As a result, to get visuals similar to Phong shading the specular shininess exponent has to be set a bit higher.
     * A general rule of thumb is to set it between 2 and 4 times the Phong shininess exponent
    */
    float specularIntensity = pow (max (dot (normal, halfwayDirection), 0.0), shininess);
    vec3 specularFactor     = specularIntensity * lightInstance.specular * specularColor;

    float distance          = length (vec4 (lightInstance.position, 1.0) - position);
    float attenuation       = 1.0 /  (lightInstance.constant  +
                                      lightInstance.linear    *  distance +
                                      lightInstance.quadratic * (distance * distance));
    ambientFactor          *= attenuation;
    diffuseFactor          *= attenuation;
    specularFactor         *= attenuation;
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
    float epsilon   = lightInstance.innerRadius - lightInstance.outerRadius;
    float theta     = dot   (lightDirection, normalize (vec4 (-lightInstance.direction, 0.0)));
    float intensity = clamp ((theta - lightInstance.outerRadius) / epsilon, 0.0, 1.0);
    /* Note that, since the shadows are rarely completely dark (due to light scattering) we leave the ambient factor out
     * of the shadow multiplications
    */
    diffuseFactor  *= shadowFactor;
    specularFactor *= shadowFactor;

    return (ambientFactor + diffuseFactor + specularFactor) * intensity;
}

void main (void) {
    vec4 normal                        = texture (normalSampler,   i_uv);
    vec4 position                      = texture (positionSampler, i_uv);
    vec3 diffuseColor                  = texture (color0Sampler,   i_uv).rgb;
    vec3 specularColor                 = texture (color1Sampler,   i_uv).rgb;
    vec3 emissionColor                 = texture (color2Sampler,   i_uv).rgb;
    float shininess                    = texture (color2Sampler,   i_uv).a;
    /* De-normalize shininess */
    shininess                          = shininess * (MAX_SHININESS - MIN_SHININESS) + MIN_SHININESS;

    /* Contribution                    [Emission] */
    vec3 fragmentColor                 = emissionColor;
    /* Contribution                    [Sky box reflection] */
    fragmentColor                     += computeColor (normal,
                                                       position,
                                                       specularColor,
                                                       REFLECTION_INTENSITY);
    /* Contribution                    [Sun lights] */
    for (uint i = 0; i < lightTypeOffsets.spotLightsOffset; i++) {
        LightInstanceSBO lightInstance = lightInstanceSBOContainer.instances[i];
        vec4 lightDirection            = normalize (vec4 (-lightInstance.direction, 0.0));
        vec4 positionLS                = lightInstance.projectionMatrix *
                                         lightInstance.viewMatrix       *
                                         position;

        float shadowFactor             = computeShadowFactor (i,
                                                              positionLS,
                                                              false);
        fragmentColor                 += computeColor        (lightInstance,
                                                              lightDirection,
                                                              normal,
                                                              position,
                                                              diffuseColor,
                                                              specularColor,
                                                              shininess,
                                                              shadowFactor);
    }
    /* Contribution                    [Spot lights] */
    for (uint i = lightTypeOffsets.spotLightsOffset; i < lightTypeOffsets.pointLightsOffset; i++) {
        LightInstanceSBO lightInstance = lightInstanceSBOContainer.instances[i];
        vec4 lightDirection            = normalize (vec4 (lightInstance.position, 1.0) - position);
        vec4 positionLS                = lightInstance.projectionMatrix *
                                         lightInstance.viewMatrix       *
                                         position;

        float shadowFactor             = computeShadowFactor (i,
                                                              positionLS,
                                                              true);
        fragmentColor                 += computeColor        (lightInstance,
                                                              lightDirection,
                                                              normal,
                                                              position,
                                                              diffuseColor,
                                                              specularColor,
                                                              shininess,
                                                              shadowFactor);
    }
    /* Contribution                    [Point lights] */
    for (uint i = lightTypeOffsets.pointLightsOffset; i < lightTypeOffsets.lightsCount; i++) {
        LightInstanceSBO lightInstance = lightInstanceSBOContainer.instances[i];
        vec4 lightDirection            = normalize (vec4 (lightInstance.position, 1.0) - position);

        float shadowFactor             = computeShadowFactor (lightInstance,
                                                              lightDirection,
                                                              normal,
                                                              position,
                                                              i - lightTypeOffsets.pointLightsOffset,
                                                              MIN_DEPTH_BIAS,
                                                              MAX_DEPTH_BIAS);
        fragmentColor                 += computeColor        (lightInstance,
                                                              lightDirection,
                                                              normal,
                                                              position,
                                                              diffuseColor,
                                                              specularColor,
                                                              shininess,
                                                              shadowFactor);
    }
    /* Final fragment color (ignore sampled alpha) */
    o_color = vec4 (fragmentColor, 1.0);
}