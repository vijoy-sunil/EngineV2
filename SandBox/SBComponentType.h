#pragma once
#include "../Backend/Common.h"
#include "SBRendererType.h"

namespace SandBox {
    /* Enums */
    typedef enum {
        LIGHT_TYPE_DIRECTIONAL,
        LIGHT_TYPE_POINT,
        LIGHT_TYPE_SPOT
    } e_lightType;

    typedef enum {
        CAMERA_TYPE_DEBUG,
        CAMERA_TYPE_SCENE
    } e_cameraType;

    typedef enum {
        PROJECTION_TYPE_PERSPECTIVE,
        PROJECTION_TYPE_ORTHOGRAPHIC
    } e_projectionType;

    typedef enum {
        TAG_TYPE_DEFAULT,
        TAG_TYPE_SKY_BOX
    } e_tagType;

    /* Components */
    struct IdComponent {
        public:
            std::string m_id   = "Unnamed";

            IdComponent (void) = default;
            IdComponent (const std::string id) {
                m_id = id;
            }
    };

    struct MeshComponent {
        public:
            /* The paths indicate the model file this mesh belongs to */
            std::string m_modelFilePath        = "Asset/Model/Debug_Cube.obj";
            std::string m_mtlFileDirPath       = "Asset/Model/";
            /* Note that the attributes are combined into one array of vertices (interleaving vertex attributes) */
            std::vector <Vertex>    m_vertices = {};
            std::vector <IndexType> m_indices  = {};

            MeshComponent (void) = default;
            MeshComponent (const std::string modelFilePath, const std::string mtlFileDirPath) {
                m_modelFilePath  = modelFilePath;
                m_mtlFileDirPath = mtlFileDirPath;
            }
    };

    struct LightComponent {
        public:
            e_lightType m_type = LIGHT_TYPE_DIRECTIONAL;
            /* The major building blocks of the Phong lighting model consist of 3 components:
             *
             * Ambient:  Even when it is dark there is usually still some light somewhere in the world, so objects are
             *           almost never completely dark. To simulate this we use an ambient lighting constant that always
             *           gives the object some color
             *
             * Diffuse:  This simulates the directional impact a light source has on an object. This is the most visually
             *           significant component of the lighting model. The more a part of an object faces the light source,
             *           the brighter it becomes
             *
             * Specular: This simulates the bright spot of a light that appears on shiny objects. Specular highlights are
             *           more inclined to the color of the light than the color of the object
            */
            glm::vec3 m_ambient  = {0.05f, 0.05f, 0.05f};
            glm::vec3 m_diffuse  = {0.40f, 0.40f, 0.40f};
            glm::vec3 m_specular = {0.50f, 0.50f, 0.50f};
            /* In the real world, lights are generally quite bright standing close by, but the brightness of a light
             * source diminishes quickly at a distance; the remaining light intensity then slowly diminishes over
             * distance. The following formula calculates an attenuation value based on a fragment's distance to the
             * light source
             *
             *                                  Fatt = 1.0 / (Kc + (Kl * d) + (Kq * d^2))
             *
             * d:  Distance from the fragment to the light source
             * Kc: The constant term is usually kept at 1.0 which is mainly there to make sure the denominator never
             *     gets smaller than 1 since it would otherwise boost the intensity with certain distances, which is not
             *     the effect we're looking for
             * Kl: The linear term is multiplied with the distance value that reduces the intensity in a linear fashion
             * Kq: The quadratic term is multiplied with the square of the distance and sets a quadratic decrease of
             *     intensity for the light source. The quadratic term will be less significant compared to the linear
             *     term when the distance is small, but gets much larger as the distance grows
             *
             * https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation shows some of the values these terms
             * could take to simulate a realistic (sort of) light source that covers a specific radius (distance). The
             * first column specifies the distance a light  will cover with the given terms. These values are good
             * starting points for most lights
             *
             * Note that, we do not set attenuation parameters for directional light
            */
            float m_constant  = 1.0f;
            float m_linear    = 0.0f;
            float m_quadratic = 0.0f;
            /* Note that, the radius values are not stored in degrees, instead we calculate the cosine value of the
             * cut off angle and pass the cosine result to the fragment shader. The reason for this is that in the
             * fragment shader we're calculating the dot product between the light direction and the spot direction and
             * the dot product returns a cosine value and not an angle; and we can't directly compare an angle with a
             * cosine value. To get the angle in the shader we then have to calculate the inverse cosine of the dot
             * product's result which is an expensive GPU operation
             *
             * So to save some performance we calculate the cosine value of a given cut off angle before hand and pass
             * this result to the fragment shader. Since both angles are now represented as cosines, we can directly
             * compare between them without expensive operations
            */
            float m_innerRadius = glm::cos (glm::radians (180.0f));
            float m_outerRadius = glm::cos (glm::radians (180.0f));

            LightComponent (void) = default;
            LightComponent (const e_lightType lightType,
                            const glm::vec3   ambient,
                            const glm::vec3   diffuse,
                            const glm::vec3   specular,
                            const float       constant,
                            const float       linear,
                            const float       quadratic,
                            const float       innerRadius,
                            const float       outerRadius) {

                m_type        = lightType;
                m_ambient     = ambient;
                m_diffuse     = diffuse;
                m_specular    = specular;

                m_constant    = constant;
                m_linear      = linear;
                m_quadratic   = quadratic;

                m_innerRadius = glm::cos (innerRadius);
                m_outerRadius = glm::cos (outerRadius);
            }
    };

    struct CameraComponent {
        public:
            e_cameraType m_cameraType         = CAMERA_TYPE_DEBUG;
            e_projectionType m_projectionType = PROJECTION_TYPE_PERSPECTIVE;
            bool m_active                     = true;
            float m_fov                       = glm::radians (60.0f);
            float m_nearPlane                 =   0.1f;
            float m_farPlane                  = 100.0f;
            float m_scale                     =   5.0f;

            CameraComponent (void) = default;
            CameraComponent (const e_cameraType cameraType,
                             const e_projectionType projectionType,
                             const bool active,
                             const float fov,
                             const float nearPlane,
                             const float farPlane,
                             const float scale) {

                m_cameraType     = cameraType;
                m_projectionType = projectionType;
                m_active         = active;
                m_fov            = fov;
                m_nearPlane      = nearPlane;
                m_farPlane       = farPlane;
                m_scale          = scale;
            }

            glm::mat4 createProjectionMatrix (const float aspectRatio) {
                glm::mat4 projectionMatrix;

                if (m_projectionType == PROJECTION_TYPE_PERSPECTIVE)
                    projectionMatrix = glm::perspective (
                        m_fov,
                        aspectRatio,
                        m_nearPlane,
                        m_farPlane
                    );
                if (m_projectionType == PROJECTION_TYPE_ORTHOGRAPHIC)
                    projectionMatrix = glm::ortho (
                        -m_scale * aspectRatio,
                         m_scale * aspectRatio,
                        -m_scale,
                         m_scale,
                         m_nearPlane,
                         m_farPlane
                    );
                /* GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates are inverted.
                 * The easiest way to compensate for that is to flip the sign on the scaling factor of the Y axis in the
                 * projection matrix. If you don't do this, then the image will be rendered upside down
                */
                projectionMatrix[1][1] *= -1;
                return projectionMatrix;
            }
    };

    struct TransformComponent {
        private:
            glm::vec3 m_xAxis = {1.0f, 0.0f, 0.0f};
            glm::vec3 m_yAxis = {0.0f, 1.0f, 0.0f};
            glm::vec3 m_zAxis = {0.0f, 0.0f, 1.0f};

        public:
            /*              +Z                                  +Y
             *              |                                   |
             *              | Blender                           |  /Camera view direction -Z
             *              |                                   | /
             *             (o)-----------| +X                  (o)-----------| +X
             *             /                                   /
             *            /                                   /
             *           /                                   /
             *         -Y                                 +Z
             *
             * Export settings
             * Forward -Z
             * Up      +Y
            */
            glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
            glm::vec3 m_rotation = {0.0f, 0.0f, 0.0f};
            glm::vec3 m_scale    = {1.0f, 1.0f, 1.0f};

            TransformComponent (void) = default;
            TransformComponent (const glm::vec3 position,
                                const glm::vec3 rotation,
                                const glm::vec3 scale = glm::vec3 (1.0f)) {

                m_position = position;
                m_rotation = rotation;
                m_scale    = scale;
            }

            glm::mat4 createModelMatrix (void) {
                glm::quat pitch       = glm::angleAxis (m_rotation.x, m_xAxis);
                glm::quat yaw         = glm::angleAxis (m_rotation.y, m_yAxis);
                glm::quat roll        = glm::angleAxis (m_rotation.z, m_zAxis);
                glm::quat orientation = glm::normalize (yaw * pitch * roll);

                return glm::translate (glm::mat4 (1.0f), m_position) *
                       glm::mat4_cast (orientation)                  *
                       glm::scale     (glm::mat4 (1.0f), m_scale);
            }

            glm::vec3 createForwardVector (void) {
                glm::quat pitch       = glm::angleAxis (m_rotation.x, m_xAxis);
                glm::quat yaw         = glm::angleAxis (m_rotation.y, m_yAxis);
                glm::quat roll        = glm::angleAxis (m_rotation.z, m_zAxis);
                glm::quat orientation = glm::normalize (yaw * pitch * roll);
                /* An entity with no rotations applied will have its forward vector pointing at -Z axis */
                return orientation * -m_zAxis;
            }
    };

    struct TextureIdxLUTComponent {
        public:
            /* Note that, we will be limiting the texture idx range to [0, 255]. This allows us to pack a total of 256
             * texture indices in 256 bytes of data in the LUT array as shown below
             *
             *      0        1        2       3              63 => write index
             * |--------|--------|--------|--------|     |--------|
             * |   32b  |   32b  |   32b  |   32b  |.....|   32b  |
             * |--------|--------|--------|--------|     |--------|
             *              |
             *              |
             *              v
             *              3        2        1        0 => offset
             *          |--------|--------|--------|--------|
             *          |   8b   |   8b   |   8b   |   8b   |
             *          |--------|--------|--------|--------|
             *             idx 7    idx 6    idx 5    idx 4 => texture index
            */
            uint32_t m_LUT[64] = {};

            TextureIdxLUTComponent (void) = default;
            void encodeTextureIdx  (const TextureIdxType oldTextureIdx, const TextureIdxType newTextureIdx) {
                uint32_t writeIdx = oldTextureIdx / 4;
                uint32_t offset   = oldTextureIdx % 4;
                uint32_t mask     = UINT8_MAX << offset * 8;

                auto& packet      = m_LUT[writeIdx];
                packet            = packet & ~mask;
                packet            = packet | (newTextureIdx << offset * 8);
            }

            TextureIdxType decodeTextureIdx (const TextureIdxType oldTextureIdx) {
                uint32_t readIdx = oldTextureIdx / 4;
                uint32_t offset  = oldTextureIdx % 4;
                uint32_t mask    = UINT8_MAX << offset * 8;

                uint32_t packet  = m_LUT[readIdx];
                packet           = packet & mask;
                return packet >> offset * 8;
            }

            void fillTextureIdxLUT (const TextureIdxType newTextureIdx) {
                uint32_t packet = newTextureIdx | (newTextureIdx << 8)  |
                                                  (newTextureIdx << 16) |
                                                  (newTextureIdx << 24);
                std::fill (
                    std::begin (m_LUT),
                    std::end   (m_LUT),
                    packet
                );
            }

            void copyToTextureIdxLUT (uint32_t* dstTextureIdxLUT) {
                std::copy (
                    std::begin (m_LUT),
                    std::end   (m_LUT),
                    dstTextureIdxLUT
                );
            }
    };

    struct RenderComponent {
        public:
            e_tagType m_tag             = TAG_TYPE_DEFAULT;
            uint32_t m_firstIndexIdx    = 0;
            uint32_t m_indicesCount     = 0;
            int32_t  m_vertexOffset     = 0;
            uint32_t m_firstInstanceIdx = 0;
            uint32_t m_instancesCount   = 1;

            RenderComponent (void) = default;
            RenderComponent (const e_tagType tag, const uint32_t instancesCount) {
                m_tag            = tag;
                m_instancesCount = instancesCount;
            }
    };
}   // namespace SandBox