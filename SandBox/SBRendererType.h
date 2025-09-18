#pragma once
#include "../Backend/Common.h"

namespace SandBox {
    /* Note that, it is possible to use either uint16_t or uint32_t for your index buffer depending on the number of
     * vertices. Additionally, you have to specify the correct type when binding the index buffer
    */
    using IndexType                    = uint32_t;
    const uint32_t g_maxFramesInFlight = 2;

    struct Vertex {
        struct Meta {
            glm::vec2 uv;
            glm::vec3 normal;
            glm::vec3 position;
        } meta;

        struct Material {
            uint32_t diffuseTextureIdx;
            uint32_t specularTextureIdx;
            uint32_t emissionTextureIdx;
            /* Shininess impacts the scattering/radius of the specular highlight. The table in the following link shows a
             * list of material properties that simulate real materials found in the outside world. Note that, the higher
             * the shininess value, the more it properly reflects the light instead of scattering it all around and thus
             * the smaller the highlight becomes http://devernay.free.fr/cours/opengl/materials.html
            */
            uint32_t shininess;
        } material;

        bool operator == (const Vertex& other) const {
            return meta.uv                     == other.meta.uv                     &&
                   meta.normal                 == other.meta.normal                 &&
                   meta.position               == other.meta.position               &&
                   material.diffuseTextureIdx  == other.material.diffuseTextureIdx  &&
                   material.specularTextureIdx == other.material.specularTextureIdx &&
                   material.emissionTextureIdx == other.material.emissionTextureIdx &&
                   material.shininess          == other.material.shininess;
        }
    };

    /* Alignment rules
     * - Scalars must be aligned by  N (= 4  bytes given 32 bit floats)
     * - Vec3    must be aligned by 4N (= 16 bytes)
     * - Mat4    must be aligned by 4N (= 16 bytes)
    */
    /* UBO - Uniform buffer object */
    struct MeshInstanceUBO {
        glm::mat4 modelMatrix;
    };

    struct ShadowConfigUBO {
        float minShadowFactor;
        float minDepthBias;                 /* Only for point lights */
        float maxDepthBias;                 /* Only for point lights */
    };

    /* PC - Push constant */
    struct ActiveLightPC {
        glm::vec3 position;
        float farPlane;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };

    struct LightTypeOffsetsPC {
        uint32_t spotLightsOffset;
        uint32_t pointLightsOffset;
        uint32_t lightsCount;
    };

    struct ActiveCameraPC {
        glm::vec3 position;
        alignas (16) glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    };
}   // namespace SandBox

namespace std {
    template <>
    struct hash <SandBox::Vertex> {
        /* https://stackoverflow.com/questions/1646807/quick-and-simple-hash-code-combinations/1646913#1646913 */
        size_t operator () (const SandBox::Vertex& vertex) const {
            size_t h1   = hash <glm::vec2>() (vertex.meta.uv);
            size_t h2   = hash <glm::vec3>() (vertex.meta.normal);
            size_t h3   = hash <glm::vec3>() (vertex.meta.position);
            size_t h4   = hash <uint32_t> () (vertex.material.diffuseTextureIdx);
            size_t h5   = hash <uint32_t> () (vertex.material.specularTextureIdx);
            size_t h6   = hash <uint32_t> () (vertex.material.emissionTextureIdx);
            size_t h7   = hash <uint32_t> () (vertex.material.shininess);

            size_t hash = 17;
            hash        = hash * 31 + h1;
            hash        = hash * 31 + h2;
            hash        = hash * 31 + h3;
            hash        = hash * 31 + h4;
            hash        = hash * 31 + h5;
            hash        = hash * 31 + h6;
            hash        = hash * 31 + h7;
            return hash;
        }
    };
}   // namespace std