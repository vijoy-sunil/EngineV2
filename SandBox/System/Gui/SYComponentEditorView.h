#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKImage.h"
#include "../../Backend/Renderer/VKSampler.h"
#include "../../Backend/Renderer/VKGui.h"
#include "../../../Backend/Scene/SNType.h"
#include "SYGuiHelper.h"
#include "../../SBComponentType.h"

namespace SandBox {
    class SYComponentEditorView: public Scene::SNSystemBase {
        private:
            struct ComponentEditorViewInfo {
                struct Meta {
                    const char* formatSpecifier;
                    float dragSpeed;
                } meta;

                struct Style {
                    float tabButtonRounding;
                    ImVec2 imageSize;
                    ImVec2 tabButtonSize;
                    ImVec2 childWindowSpacing;
                    ImVec4 tabButtonInactiveColor;
                    ImVec4 tabButtonActiveColor;
                } style;

                struct Flag {
                    ImGuiColorEditFlags colorEditFlags;
                } flag;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                    ImTextureID textureId;
                } resource;
            } m_componentEditorViewInfo;

        public:
            SYComponentEditorView (void) {
                m_componentEditorViewInfo = {};

                auto& logObj = m_componentEditorViewInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initComponentEditorViewInfo (Scene::SNImpl* sceneObj,
                                              Collection::CNImpl* collectionObj) {

                m_componentEditorViewInfo.meta.formatSpecifier         = "%0.4f";
                m_componentEditorViewInfo.meta.dragSpeed               = 0.1f;

                m_componentEditorViewInfo.style.tabButtonRounding      = 0.0f;
                m_componentEditorViewInfo.style.imageSize              = ImVec2 (200.0f, 200.0f);
                m_componentEditorViewInfo.style.tabButtonSize          = ImVec2 ( 48.0f,  40.0f);
                m_componentEditorViewInfo.style.childWindowSpacing     = ImVec2 (  0.0f, ImGui::GetStyle().ItemSpacing.y);
                m_componentEditorViewInfo.style.tabButtonInactiveColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
                m_componentEditorViewInfo.style.tabButtonActiveColor   = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];

                m_componentEditorViewInfo.flag.colorEditFlags          = ImGuiColorEditFlags_NoBorder   |
                                                                         ImGuiColorEditFlags_AlphaBar   |
                                                                         ImGuiColorEditFlags_DisplayRGB |
                                                                         ImGuiColorEditFlags_Float      |
                                                                         ImGuiColorEditFlags_InputRGB;

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_componentEditorViewInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                          << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& resource     = m_componentEditorViewInfo.resource;
                resource.sceneObj  = sceneObj;
                auto imageObj      = collectionObj->getCollectionTypeInstance <Renderer::VKImage>   (
                    "G_DEFAULT_TEXTURE_2"
                );
                auto samplerObj    = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> (
                    "F_LIGHT_GBUFFER"
                );
                auto guiObj        = collectionObj->getCollectionTypeInstance <Renderer::VKGui>     (
                    "DRAW_OPS"
                );
                resource.textureId = guiObj->addTexture (
                    *imageObj->getImageView(),
                    *samplerObj->getSampler(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );
            }

            void update (const Scene::Entity selectedEntity,
                         Scene::Entity& activeCameraEntity,
                         Scene::ComponentType& selectedComponentType) {

                auto& meta           = m_componentEditorViewInfo.meta;
                auto& style          = m_componentEditorViewInfo.style;
                auto& flag           = m_componentEditorViewInfo.flag;
                auto& resource       = m_componentEditorViewInfo.resource;
                auto& sceneObj       = resource.sceneObj;
                auto entitySignature = sceneObj->getEntitySignature (selectedEntity);

                /* Remove horizontal item spacing between left child and right child */
                ImGui::PushStyleVar       (ImGuiStyleVar_ItemSpacing, style.childWindowSpacing);
                if (ImGui::Begin          (ICON_FA_PEN " Component editor", nullptr, ImGuiWindowFlags_None)) {
                    if (ImGui::BeginChild ("##LeftChild",
                                           ImVec2 (style.tabButtonSize.x, 0.0f),
                                           ImGuiChildFlags_None,
                                           ImGuiWindowFlags_NoBackground)) {

                        if (ImGui::BeginTable ("##Table", 1, ImGuiTableFlags_None)) {
                            for (auto const& [type, info]: g_componentTypeToLabelInfoMap) {
                                bool buttonClicked  = false;
                                bool buttonSelected = selectedComponentType == type;
                                if (!entitySignature[type])
                                    continue;

                                ImGui::TableNextRow           (ImGuiTableRowFlags_None);
                                ImGui::TableNextColumn();

                                ImGui::PushID                 (type);
                                ImGui::PushStyleVar           (ImGuiStyleVar_FrameRounding, style.tabButtonRounding);
                                if (buttonSelected)
                                    ImGui::PushStyleColor     (ImGuiCol_Button,             style.tabButtonActiveColor);
                                else
                                    ImGui::PushStyleColor     (ImGuiCol_Button,             style.tabButtonInactiveColor);
                                ImGui::PushStyleColor         (ImGuiCol_ButtonHovered,      style.tabButtonActiveColor);
                                ImGui::PushStyleColor         (ImGuiCol_ButtonActive,       style.tabButtonActiveColor);

                                buttonClicked = ImGui::Button (info.icon,                   style.tabButtonSize);
                                ImGui::PopStyleColor          (3);
                                ImGui::PopStyleVar();
                                ImGui::PopID();
                                ImGui::SetItemTooltip         ("%s", info.label);

                                if (buttonClicked)
                                    selectedComponentType = type;
                            }
                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();
                    ImGui::SameLine();

                    if (ImGui::BeginChild ("##RightChild",
                                           ImVec2 (0.0f, 0.0f),
                                           ImGuiChildFlags_AlwaysUseWindowPadding,
                                           ImGuiWindowFlags_None)) {

                        switch (selectedComponentType) {
                            case 0:
                            {   /* Meta component type */
                                auto metaComponent  = sceneObj->getComponent <MetaComponent> (selectedEntity);
                                auto& id            = metaComponent->m_id;
                                std::string tagType = std::string (getTagTypeString (metaComponent->m_tagType));

                                ImGui::InputText     ("Id",       &id,      ImGuiInputTextFlags_None);
                                ImGui::BeginDisabled (true);
                                ImGui::InputText     ("Tag type", &tagType, ImGuiInputTextFlags_ReadOnly);
                                ImGui::EndDisabled();
                                break;
                            }
                            case 1:
                            {   /* Mesh component type */
                                auto meshComponent   = sceneObj->getComponent <MeshComponent> (selectedEntity);
                                auto& loadPending    = meshComponent->m_loadPending;
                                auto& modelFilePath  = meshComponent->m_modelFilePath;
                                auto& mtlFileDirPath = meshComponent->m_mtlFileDirPath;

                                ImGui::BeginDisabled (true);
                                ImGui::Checkbox      ("Load pending", &loadPending);

                                ImGui::SeparatorText ("Path");
                                ImGui::InputText     ("Model file",   &modelFilePath,  ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputText     ("Mtl file dir", &mtlFileDirPath, ImGuiInputTextFlags_ReadOnly);
                                ImGui::EndDisabled();
                                break;
                            }
                            case 2:
                            {   /* Light component type */
                                auto lightComponent   = sceneObj->getComponent <LightComponent> (selectedEntity);
                                std::string lightType = std::string (getLightTypeString (lightComponent->m_lightType));
                                auto& ambient         = lightComponent->m_ambient;
                                auto& diffuse         = lightComponent->m_diffuse;
                                auto& specular        = lightComponent->m_specular;
                                auto& constant        = lightComponent->m_constant;
                                auto& linear          = lightComponent->m_linear;
                                auto& quadratic       = lightComponent->m_quadratic;
                                auto& innerRadius     = lightComponent->m_innerRadius;
                                auto& outerRadius     = lightComponent->m_outerRadius;
                                auto& nearPlane       = lightComponent->m_nearPlane;
                                auto& farPlane        = lightComponent->m_farPlane;
                                auto& scale           = lightComponent->m_scale;

                                float innerRadiusDeg  = glm::degrees (glm::acos (innerRadius));
                                float outerRadiusDeg  = glm::degrees (glm::acos (outerRadius));

                                ImGui::BeginDisabled (true);
                                ImGui::InputText     ("Light type", &lightType,      ImGuiInputTextFlags_ReadOnly);
                                ImGui::EndDisabled();

                                ImGui::SeparatorText ("Color");
                                ImGui::ColorEdit3    ("Ambient",    glm::value_ptr   (ambient),  flag.colorEditFlags);
                                ImGui::ColorEdit3    ("Diffuse",    glm::value_ptr   (diffuse),  flag.colorEditFlags);
                                ImGui::ColorEdit3    ("Specular",   glm::value_ptr   (specular), flag.colorEditFlags);

                                ImGui::SeparatorText ("Attenuation");
                                ImGui::DragFloat     ("Constant",   &constant,       meta.dragSpeed,
                                                      1.0f,         FLT_MAX,         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                /* Finer drag speed */
                                ImGui::DragFloat     ("Linear",     &linear,         meta.dragSpeed / 1000.0f,
                                                      0.0f,         FLT_MAX,         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Quadratic",  &quadratic,      meta.dragSpeed / 1000.0f,
                                                      0.0f,         FLT_MAX,         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                ImGui::SeparatorText ("Radius");
                                createHelpMarker     ("Use only for spot lights");
                                ImGui::DragFloat     ("Inner",      &innerRadiusDeg, meta.dragSpeed,
                                                      0.0f,         180.0f,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Outer",      &outerRadiusDeg, meta.dragSpeed,
                                                      0.0f,         180.0f,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                ImGui::SeparatorText ("Frustum");
                                ImGui::DragFloat     ("Near plane", &nearPlane,      meta.dragSpeed,
                                                      0.0f,         FLT_MAX,         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Far plane",  &farPlane,       meta.dragSpeed,
                                                      0.0f,         FLT_MAX,         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Scale",      &scale,          meta.dragSpeed,
                                                      0.0f,         FLT_MAX,         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                innerRadius = glm::cos (glm::radians (innerRadiusDeg));
                                outerRadius = glm::cos (glm::radians (outerRadiusDeg));
                                break;
                            }
                            case 3:
                            {   /* Camera component type */
                                auto cameraComponent       = sceneObj->getComponent <CameraComponent> (selectedEntity);
                                std::string projectionType = std::string (getProjectionTypeString (
                                    cameraComponent->m_projectionType
                                ));
                                auto& fov                  = cameraComponent->m_fov;
                                auto& nearPlane            = cameraComponent->m_nearPlane;
                                auto& farPlane             = cameraComponent->m_farPlane;
                                auto& scale                = cameraComponent->m_scale;

                                float fovDeg               = glm::degrees (fov);
                                bool checkBoxSelected      = selectedEntity == activeCameraEntity;

                                ImGui::BeginDisabled (true);
                                ImGui::InputText     ("Projection type", &projectionType,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::EndDisabled();

                                ImGui::Checkbox      ("Active",          &checkBoxSelected);

                                ImGui::SeparatorText ("Frustum");
                                ImGui::DragFloat     ("Fov",             &fovDeg,    meta.dragSpeed,
                                                      0.0f,              180.0f,     meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Near plane",      &nearPlane, meta.dragSpeed,
                                                      0.0f,              FLT_MAX,    meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Far plane",       &farPlane,  meta.dragSpeed,
                                                      0.0f,              FLT_MAX,    meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Scale",           &scale,     meta.dragSpeed,
                                                      0.0f,              FLT_MAX,    meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                fov = glm::radians (fovDeg);
                                if (checkBoxSelected)
                                    activeCameraEntity = selectedEntity;
                                break;
                            }
                            case 4:
                            {   /* Transform component type */
                                auto transformComponent   = sceneObj->getComponent <TransformComponent> (selectedEntity);
                                auto& position            = transformComponent->m_position;
                                auto& orientation         = transformComponent->m_orientation;
                                auto& scale               = transformComponent->m_scale;

                                glm::vec3 initialRotation = glm::degrees (glm::eulerAngles (orientation));
                                glm::vec3 finalRotation   = initialRotation;
                                glm::vec3 rotationDeltas  = glm::vec3 (0.0f);

                                ImGui::SeparatorText ("Position");
                                ImGui::PushID        (0);
                                ImGui::DragFloat     ("X" ,     &position.x,      meta.dragSpeed,
                                                      -FLT_MAX, FLT_MAX,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Y",      &position.y,      meta.dragSpeed,
                                                      -FLT_MAX, FLT_MAX,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Z",      &position.z,      meta.dragSpeed,
                                                      -FLT_MAX, FLT_MAX,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::PopID();

                                ImGui::SeparatorText ("Rotation");
                                ImGui::DragFloat     ("Pitch",  &finalRotation.x, meta.dragSpeed,
                                                      -180.0f,  180.0f,           meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Yaw",    &finalRotation.y, meta.dragSpeed,
                                                      -180.0f,  180.0f,           meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Roll",   &finalRotation.z, meta.dragSpeed,
                                                      -180.0f,  180.0f,           meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                ImGui::SeparatorText ("Scale");
                                ImGui::PushID        (1);
                                ImGui::DragFloat     ("X",      &scale.x,         meta.dragSpeed,
                                                      0.0f,     FLT_MAX,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Y",      &scale.y,         meta.dragSpeed,
                                                      0.0f,     FLT_MAX,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Z",      &scale.z,         meta.dragSpeed,
                                                      0.0f,     FLT_MAX,          meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::PopID();

                                /* Compute delta */
                                rotationDeltas = glm::radians (finalRotation - initialRotation);

                                if (rotationDeltas.y != 0.0f)   transformComponent->addYaw   (rotationDeltas.y);
                                if (rotationDeltas.x != 0.0f)   transformComponent->addPitch (rotationDeltas.x);
                                if (rotationDeltas.z != 0.0f)   transformComponent->addRoll  (rotationDeltas.z);
                                break;
                            }
                            case 5:
                            {   /* Texture idx offset component type */
                                auto textureIdxOffsetComponent
                                              = sceneObj->getComponent <TextureIdxOffsetComponent> (selectedEntity);
                                auto& offset0 = textureIdxOffsetComponent->m_offsets[0];
                                auto& offset1 = textureIdxOffsetComponent->m_offsets[1];
                                auto& offset2 = textureIdxOffsetComponent->m_offsets[2];

                                ImGui::InputInt ("Offset 0", &offset0, 1, 2, ImGuiInputTextFlags_None);
                                ImGui::InputInt ("Offset 1", &offset1, 1, 2, ImGuiInputTextFlags_None);
                                ImGui::InputInt ("Offset 2", &offset2, 1, 2, ImGuiInputTextFlags_None);
                                break;
                            }
                            case 6:
                            {   /* Color component type */
                                auto colorComponent = sceneObj->getComponent <ColorComponent> (selectedEntity);
                                auto& color         = colorComponent->m_color;

                                ImGui::ColorPicker4 ("Color", glm::value_ptr (color), flag.colorEditFlags);
                                break;
                            }
                            case 7:
                            {   /* Render component type */
                                auto renderComponent   = sceneObj->getComponent <RenderComponent> (selectedEntity);
                                auto& firstIndexIdx    = renderComponent->m_firstIndexIdx;
                                auto& indicesCount     = renderComponent->m_indicesCount;
                                auto& vertexOffset     = renderComponent->m_vertexOffset;
                                auto& firstInstanceIdx = renderComponent->m_firstInstanceIdx;
                                auto& instancesCount   = renderComponent->m_instancesCount;

                                ImGui::BeginDisabled (true);
                                ImGui::InputInt      ("First index idx",    reinterpret_cast <int*> (&firstIndexIdx),
                                                      0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputInt      ("Indices count",      reinterpret_cast <int*> (&indicesCount),
                                                      0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputInt      ("Vertex offset",      &vertexOffset,
                                                      0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputInt      ("First instance idx", reinterpret_cast <int*> (&firstInstanceIdx),
                                                      0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputInt      ("Instances count",    reinterpret_cast <int*> (&instancesCount),
                                                      0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::EndDisabled();
                                break;
                            }
                            case 8:
                                /* Std no alpha tag component type */
                            case 9:
                                /* Std alpha tag component type */
                            case 10:
                                /* Wire tag component type */
                            case 11:
                            {   /* Sky box tag component type */
                                float windowContentWidth = ImGui::GetWindowContentRegionMax().x -
                                                           ImGui::GetWindowContentRegionMin().x;
                                const char* text         = "No info";
                                float textWidth          = ImGui::CalcTextSize (text).x;

                                /* Center align image and text */
                                ImGui::SetCursorPosX ((windowContentWidth - style.imageSize.x) * 0.5f);
                                ImGui::Image         (resource.textureId, style.imageSize);

                                ImGui::SetCursorPosX ((windowContentWidth - textWidth) * 0.5f);
                                ImGui::Text          ("%s", text);
                                break;
                            }
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::PopStyleVar();
                ImGui::End();
            }

            ~SYComponentEditorView (void) {
                delete m_componentEditorViewInfo.resource.logObj;
            }
    };
}   // namespace SandBox