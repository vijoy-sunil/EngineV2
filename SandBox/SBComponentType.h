#pragma once
#include "../Backend/Common.h"
#include "SBRendererType.h"

namespace SandBox {
    typedef enum {
        LIGHT_TYPE_SUN,
        LIGHT_TYPE_SPOT,
        LIGHT_TYPE_POINT
    } e_lightType;

    typedef enum {
        PROJECTION_TYPE_PERSPECTIVE,
        PROJECTION_TYPE_ORTHOGRAPHIC
    } e_projectionType;

    typedef enum {
        TAG_TYPE_NONE,
        TAG_TYPE_STD_NO_ALPHA,
        TAG_TYPE_STD_ALPHA,
        TAG_TYPE_WIRE,
        TAG_TYPE_SKY_BOX
    } e_tagType;

    const char* getLightTypeString (const e_lightType lightType) {
        switch (lightType) {
            case LIGHT_TYPE_SUN:                    return "LIGHT_TYPE_SUN";
            case LIGHT_TYPE_SPOT:                   return "LIGHT_TYPE_SPOT";
            case LIGHT_TYPE_POINT:                  return "LIGHT_TYPE_POINT";
            default:                                return "UNDEFINED";
        }
    }

    const char* getProjectionTypeString (const e_projectionType projectionType) {
        switch (projectionType) {
            case PROJECTION_TYPE_PERSPECTIVE:       return "PROJECTION_TYPE_PERSPECTIVE";
            case PROJECTION_TYPE_ORTHOGRAPHIC:      return "PROJECTION_TYPE_ORTHOGRAPHIC";
            default:                                return "UNDEFINED";
        }
    }

    const char* getTagTypeString (const e_tagType tagType) {
        switch (tagType) {
            case TAG_TYPE_NONE:                     return "TAG_TYPE_NONE";
            case TAG_TYPE_STD_NO_ALPHA:             return "TAG_TYPE_STD_NO_ALPHA";
            case TAG_TYPE_STD_ALPHA:                return "TAG_TYPE_STD_ALPHA";
            case TAG_TYPE_WIRE:                     return "TAG_TYPE_WIRE";
            case TAG_TYPE_SKY_BOX:                  return "TAG_TYPE_SKY_BOX";
            default:                                return "UNDEFINED";
        }
    }

    /* Components */
    struct MetaComponent {
        public:
            std::string m_id     = "UNDEFINED";
            e_tagType m_tagType  = TAG_TYPE_NONE;

            MetaComponent (void) = default;
            MetaComponent (const std::string id, const e_tagType tagType) {
                m_id             = id;
                m_tagType        = tagType;
            }
    };

    struct MeshComponent {
        public:
            bool m_loadPending                 = true;
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

            /* Manually populate vertices and indices */
            MeshComponent (const std::vector <Vertex> vertices, const std::vector <IndexType> indices) {
                m_loadPending    = false;
                m_modelFilePath  = "";
                m_mtlFileDirPath = "";
                m_vertices       = vertices;
                m_indices        = indices;
            }
    };

    struct LightComponent {
        public:
            e_lightType m_lightType = LIGHT_TYPE_SUN;
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
             * Note that, we do not set attenuation parameters for sun light
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
            /* Note that, because a projection matrix indirectly determines the range of what is visible (or, what is not
             * clipped) you want to make sure the size of the projection frustum correctly contains the objects you want
             * to be in the shadow image. When objects or fragments are not included in the frustum they will not produce
             * shadows. In short, you want to match the projection frustum more closely with your attenuated light volume
            */
            float m_nearPlane   =  0.01f;
            float m_farPlane    = 100.0f;
            float m_scale       =   5.0f;

            LightComponent (void) = default;
            LightComponent (const e_lightType lightType,
                            const glm::vec3   ambient,
                            const glm::vec3   diffuse,
                            const glm::vec3   specular,
                            const float       constant,
                            const float       linear,
                            const float       quadratic,
                            const float       innerRadius,
                            const float       outerRadius,
                            const float       nearPlane,
                            const float       farPlane) {

                m_lightType   = lightType;
                m_ambient     = ambient;
                m_diffuse     = diffuse;
                m_specular    = specular;

                m_constant    = constant;
                m_linear      = linear;
                m_quadratic   = quadratic;

                m_innerRadius = glm::cos (innerRadius);
                m_outerRadius = glm::cos (outerRadius);
                m_nearPlane   = nearPlane;
                m_farPlane    = farPlane;
            }

            /*  Coordinate systems
             *  +---------------+---------------+---------------+---------------+---------------+---------------+
             *  |     Model     |     World     |     View      |     Clip      |     NDC       |     Window    |
             *  |     space     |     space     |     space     |     space     |               |     space     |
             *  +-------:-------+-------:-------+-------:-------+-------:-------+-------:-------+-------:-------+
             *          :               :               :               :               :               :
             *          :------->-------:------->-------:------->-------:------->-------:------->-------:
             *                  Model           View            Projection      Cliiping +      View port
             *                  matrix          matrix          matrix          Perspective     transformation
             *                                                                  divide
             *
             * Note that, the clipping and perspective divide operations are fixed function (hardwired) operations. Each
             * graphics API specifies the coordinate systems in which these happen. All we need to do is create the model,
             * view and projection matrices to get our vertex data "correctly into clip space"
             *
             * With opengl, the fixed function parts of the pipeline all use left-handed coordinate systems. If we stick
             * with the common conventions of using a right-handed set of coordinate systems for model, world and view
             * space, then the transformation from view space to clip space must flip the handedness of the coordinate
             * system somehow
             *
             * The glm::perspective method not only performs the perspective projection transform but it also bakes in
             * the flip from right-handed coordinates to left-handed coordinates by flipping the z axis and using -z as
             * the w component causing the change in handedness
             *
             * Conversely to opengl, the fixed function coordinate systems used in vulkan remain as right-handed as shown
             * below. Note that, even though z increases into the distance and y is increasing down, it is still in fact
             * a right-handed coordinate system
             *
             *              +Y                                      +Z
             *              |                                      /
             *              |   View                              /   Clip
             *              |   space                            /    space
             *             (o)-----------| +X                  (o)-----------| +X
             *             /                                    |
             *            /                                     |
             *           /                                      |
             *           +Z                                     +Y
             *
             * We need to reorient our coordinate axes from view space (x-right, y-up, looking down the -z axis) to the
             * vulkan clip space (x-right, y-down, looking down the +z axis), which can be achieved by flipping both the
             * z (baked in glm::perspective) and y axes, or, perform a rotation of 180 degrees about the x axis
            */
            glm::mat4 createProjectionMatrix (const float aspectRatio) {
                glm::mat4 projectionMatrix;
                switch (m_lightType) {
                    case LIGHT_TYPE_SUN:
                        projectionMatrix = glm::ortho       (-m_scale * aspectRatio,
                                                              m_scale * aspectRatio,
                                                             -m_scale,
                                                              m_scale,
                                                              m_nearPlane,
                                                              m_farPlane);
                        break;
                    case LIGHT_TYPE_SPOT:
                        projectionMatrix = glm::perspective (2.0f * glm::acos (m_outerRadius),
                                                             aspectRatio,
                                                             m_nearPlane,
                                                             m_farPlane);
                        break;
                    case LIGHT_TYPE_POINT:
                        /* Note that, we set the fov to 90 degrees to make sure the viewing field is exactly large enough
                         * to fill a single face of the cube map such that all faces align correctly to each other at the
                         * edges
                        */
                        projectionMatrix = glm::perspective (glm::radians (90.0f),
                                                             aspectRatio,
                                                             m_nearPlane,
                                                             m_farPlane);
                        break;
                };
                projectionMatrix[1][1] *= -1.0f;
                return projectionMatrix;
            }
    };

    struct CameraComponent {
        public:
            e_projectionType m_projectionType = PROJECTION_TYPE_PERSPECTIVE;
            bool m_active                     = true;
            float m_fov                       = glm::radians (60.0f);
            float m_nearPlane                 =  0.01f;
            float m_farPlane                  = 100.0f;
            float m_scale                     =   5.0f;

            CameraComponent (void) = default;
            CameraComponent (const e_projectionType projectionType,
                             const bool active,
                             const float fov,
                             const float nearPlane,
                             const float farPlane) {

                m_projectionType = projectionType;
                m_active         = active;
                m_fov            = fov;
                m_nearPlane      = nearPlane;
                m_farPlane       = farPlane;
            }

            glm::mat4 createProjectionMatrix (const float aspectRatio) {
                glm::mat4 projectionMatrix;
                switch (m_projectionType) {
                    case PROJECTION_TYPE_PERSPECTIVE:
                        projectionMatrix = glm::perspective (m_fov,
                                                             aspectRatio,
                                                             m_nearPlane,
                                                             m_farPlane);
                        break;
                    case PROJECTION_TYPE_ORTHOGRAPHIC:
                        projectionMatrix = glm::ortho       (-m_scale * aspectRatio,
                                                              m_scale * aspectRatio,
                                                             -m_scale,
                                                              m_scale,
                                                              m_nearPlane,
                                                              m_farPlane);
                        break;
                };
                projectionMatrix[1][1] *= -1.0f;
                return projectionMatrix;
            }
    };

    struct TransformComponent {
        private:
            /* Global axes */
            glm::vec3 m_xAxis         = {1.0f, 0.0f, 0.0f};
            glm::vec3 m_yAxis         = {0.0f, 1.0f, 0.0f};
            glm::vec3 m_zAxis         = {0.0f, 0.0f, 1.0f};
            /* Local axes */
            glm::vec3 m_rightVector   =  m_xAxis;
            glm::vec3 m_upVector      =  m_yAxis;
            glm::vec3 m_forwardVector = -m_zAxis;

            void updateLocalAxes (void) {
                m_rightVector   = glm::normalize (m_orientation *  m_xAxis);
                m_upVector      = glm::normalize (m_orientation *  m_yAxis);
                m_forwardVector = glm::normalize (m_orientation * -m_zAxis);
            }

        public:
            /* Right-handed coordinate system
             *              +Z                                  +Y
             *              |                                   |
             *              | Blender                           |  /Camera view direction -Z
             *              |                                   | /
             *             (o)-----------| +X                  (o)-----------| +X
             *             /                                   /
             *            /                                   /
             *           /                                   /
             *           -Y                                  +Z
             *
             * Export settings
             * Forward -Z
             * Up      +Y
            */
            glm::vec3 m_position    = {0.0f, 0.0f, 0.0f};
            glm::quat m_orientation = {1.0f, 0.0f, 0.0f, 0.0f};     /* Identity quaternion */
            glm::vec3 m_scale       = {1.0f, 1.0f, 1.0f};

            TransformComponent (void) = default;
            TransformComponent (const glm::vec3 position,
                                const glm::vec3 rotation,
                                const glm::vec3 scale = glm::vec3 (1.0f)) {

                m_position = position;
                m_scale    = scale;
                /* Yaw->Pitch->Roll */
                addYaw   (rotation.y);
                addPitch (rotation.x);
                addRoll  (rotation.z);
            }

            glm::vec3 getRightVector (void) {
                return m_rightVector;
            }

            glm::vec3 getUpVector (void) {
                return m_upVector;
            }

            glm::vec3 getForwardVector (void) {
                return m_forwardVector;
            }

            void addPitch (const float val) {
                glm::quat pitch = glm::angleAxis (val, m_rightVector);
                m_orientation   = glm::normalize (pitch * m_orientation);
                updateLocalAxes();
            }

            void addYaw (const float val) {
                glm::quat yaw   = glm::angleAxis (val, m_upVector);
                m_orientation   = glm::normalize (yaw * m_orientation);
                updateLocalAxes();
            }

            void addRoll (const float val) {
                glm::quat roll  = glm::angleAxis (val, m_forwardVector);
                m_orientation   = glm::normalize (roll * m_orientation);
                updateLocalAxes();
            }

            glm::mat4 createModelMatrix (void) {
                return glm::translate (glm::mat4 (1.0f), m_position) *
                       glm::mat4_cast (m_orientation)                *
                       glm::scale     (glm::mat4 (1.0f), m_scale);
            }
    };

    struct TextureIdxOffsetComponent {
        public:
            /* Note that, the offsets are intended for meshes with only one material */
            int32_t m_offsets[3] = {0, 0, 0};

            TextureIdxOffsetComponent (void) = default;
            TextureIdxOffsetComponent (const int32_t diffuseTextureIdxOffset,
                                       const int32_t specularTextureIdxOffset,
                                       const int32_t emissionTextureIdxOffset) {

                m_offsets[0] = diffuseTextureIdxOffset;
                m_offsets[1] = specularTextureIdxOffset;
                m_offsets[2] = emissionTextureIdxOffset;
            }

            void copyTo (int32_t* dstOffsets) {
                std::copy (
                    std::begin (m_offsets),
                    std::end   (m_offsets),
                    dstOffsets
                );
            }
    };

    struct ColorComponent {
        public:
            glm::vec4 m_color = glm::vec4 (1.0f);

            ColorComponent (void) = default;
            ColorComponent (const glm::vec4 color) {
                m_color = color;
            }
    };

    struct RenderComponent {
        public:
            uint32_t m_firstIndexIdx    = 0;
            uint32_t m_indicesCount     = 0;
            int32_t  m_vertexOffset     = 0;
            uint32_t m_firstInstanceIdx = 0;
            uint32_t m_instancesCount   = 1;

            RenderComponent (void) = default;
            RenderComponent (const uint32_t instancesCount) {
                m_instancesCount = instancesCount;
            }
    };

    struct StdNoAlphaTagComponent {
        public:
            StdNoAlphaTagComponent (void) = default;
    };

    struct StdAlphaTagComponent {
        public:
            StdAlphaTagComponent (void) = default;
    };

    struct WireTagComponent {
        public:
            WireTagComponent (void) = default;
    };

    struct SkyBoxTagComponent {
        public:
            SkyBoxTagComponent (void) = default;
    };
}   // namespace SandBox