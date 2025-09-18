#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKImage.h"
#include "../../Backend/Renderer/VKSampler.h"
#include "../../Backend/Renderer/VKGui.h"
#include "../../SBTexturePool.h"
#include "SYGuiHelper.h"
#include "../SYConfig.h"

namespace SandBox {
    class SYConfigView: public Scene::SNSystemBase {
        private:
            struct ConfigViewInfo {
                struct Meta {
                    const char* formatSpecifier;
                    float dragSpeed;
                    /* Note that, we are using `int` for texture idx for ImGui::InputInt compatibility */
                    int selectedTextureIdx;
                    size_t selectedConfigIdx;
                    std::unordered_map <size_t, LabelInfo> configIdxToLabelInfoMap;
                    std::vector <ImTextureID> textureIds;
                } meta;

                struct Style {
                    float tabButtonRounding;
                    ImVec2 imageSize;
                    ImVec2 tabButtonSize;
                    ImVec2 childWindowSpacing;
                    ImVec4 tabButtonInactiveColor;
                    ImVec4 tabButtonActiveColor;
                } style;

                struct Resource {
                    Log::LGImpl* logObj;
                    SBTexturePool* texturePoolObj;
                } resource;
            } m_configViewInfo;

        public:
            SYConfigView (void) {
                m_configViewInfo = {};

                auto& logObj = m_configViewInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initConfigViewInfo (Collection::CNImpl* collectionObj,
                                     SBTexturePool* texturePoolObj) {

                auto& meta                   = m_configViewInfo.meta;
                auto& style                  = m_configViewInfo.style;
                auto& resource               = m_configViewInfo.resource;

                meta.formatSpecifier         = "%0.4f";
                meta.dragSpeed               = 0.01f;
                meta.selectedTextureIdx      = 0;
                meta.selectedConfigIdx       = 0;
                meta.configIdxToLabelInfoMap = {
                    {0, {"Texture pool", ICON_FA_IMAGES   }},
                    {1, {"Shadow",       ICON_FA_CLOUD_SUN}},
                    {2, {"Camera",       ICON_FA_CAMERA   }}
                };

                style.tabButtonRounding      = 0.0f;
                style.imageSize              = ImVec2 (200.0f, 200.0f);
                style.tabButtonSize          = ImVec2 ( 48.0f,  40.0f);
                style.childWindowSpacing     = ImVec2 (  0.0f, ImGui::GetStyle().ItemSpacing.y);
                style.tabButtonInactiveColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
                style.tabButtonActiveColor   = ImGui::GetStyle().Colors[ImGuiCol_ChildBg];

                if (collectionObj == nullptr || texturePoolObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.texturePoolObj      = texturePoolObj;
                auto samplerObj              = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> (
                    "F_LIGHT_GBUFFER"
                );
                auto guiObj                  = collectionObj->getCollectionTypeInstance <Renderer::VKGui>     (
                    "DRAW_OPS"
                );
                auto texturePool             = resource.texturePoolObj->getTexturePool();

                for (auto const& [idx, info]: texturePool) {
                    auto imageObj            = collectionObj->getCollectionTypeInstance <Renderer::VKImage>   (
                        "G_DEFAULT_TEXTURE_" + std::to_string (idx)
                    );
                    meta.textureIds.push_back (
                        guiObj->addTexture (*imageObj->getImageView(),
                                            *samplerObj->getSampler(),
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                    );
                }
            }

            void update (void) {
                auto& meta       = m_configViewInfo.meta;
                auto& style      = m_configViewInfo.style;
                auto texturePool = m_configViewInfo.resource.texturePoolObj->getTexturePool();

                /* Remove horizontal item spacing between left child and right child */
                ImGui::PushStyleVar       (ImGuiStyleVar_ItemSpacing, style.childWindowSpacing);
                if (ImGui::Begin          (ICON_FA_GEAR " Config", nullptr, ImGuiWindowFlags_None)) {
                    if (ImGui::BeginChild ("##LeftChild",
                                           ImVec2 (style.tabButtonSize.x, 0.0f),
                                           ImGuiChildFlags_None,
                                           ImGuiWindowFlags_NoBackground)) {

                        if (ImGui::BeginTable ("##Table", 1, ImGuiTableFlags_None)) {
                            for (auto const& [idx, info]: meta.configIdxToLabelInfoMap) {
                                bool buttonClicked  = false;
                                bool buttonSelected = meta.selectedConfigIdx == idx;

                                ImGui::TableNextRow           (ImGuiTableRowFlags_None);
                                ImGui::TableNextColumn();

                                ImGui::PushID                 (idx);
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
                                    meta.selectedConfigIdx = idx;
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

                        switch (meta.selectedConfigIdx) {
                            case 0:
                            {   /* Texture pool */
                                ImGui::InputInt ("Texture idx", &meta.selectedTextureIdx, 1, 2,
                                                 ImGuiInputTextFlags_None);

                                meta.selectedTextureIdx  = std::clamp (
                                    meta.selectedTextureIdx,
                                    0,
                                    static_cast <int> (texturePool.size()) - 1
                                );
                                float windowContentWidth = ImGui::GetWindowContentRegionMax().x -
                                                           ImGui::GetWindowContentRegionMin().x;
                                int width                = texturePool[meta.selectedTextureIdx].width;
                                int height               = texturePool[meta.selectedTextureIdx].height;
                                int channelsCount        = texturePool[meta.selectedTextureIdx].channelsCount;

                                ImGui::SeparatorText ("Info");
                                ImGui::BeginDisabled (true);
                                ImGui::InputInt      ("Width",          &width,         0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputInt      ("Height",         &height,        0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::InputInt      ("Channels count", &channelsCount, 0, 0,
                                                      ImGuiInputTextFlags_ReadOnly);
                                ImGui::EndDisabled();

                                ImGui::SeparatorText ("Preview");
                                ImGui::SetCursorPosX ((windowContentWidth - style.imageSize.x) * 0.5f);
                                ImGui::Image         (meta.textureIds[meta.selectedTextureIdx], style.imageSize);
                                break;
                            }
                            case 1:
                            {   /* Shadow */
                                auto& shadow = g_systemConfig.shadow;
                                ImGui::SeparatorText ("Shadow factor");
                                ImGui::PushID        (0);
                                ImGui::DragFloat     ("Min",      &shadow.minShadowFactor,         meta.dragSpeed,
                                                      0.0f,       1.0f,                            meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::PopID();

                                ImGui::SeparatorText ("Dpeth bias (Other)");
                                ImGui::DragFloat     ("Constant", &shadow.depthBiasConstantFactor, meta.dragSpeed,
                                                      0.0f,       FLT_MAX,                         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Slope",    &shadow.depthBiasSlopeFactor,    meta.dragSpeed,
                                                      0.0f,       FLT_MAX,                         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                ImGui::SeparatorText ("Dpeth bias (Point)");
                                ImGui::PushID        (1);
                                ImGui::DragFloat     ("Min",      &shadow.minDepthBias,            meta.dragSpeed,
                                                      0.0f,       FLT_MAX,                         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Max",      &shadow.maxDepthBias,            meta.dragSpeed,
                                                      0.0f,       FLT_MAX,                         meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::PopID();
                                break;
                            }
                            case 2:
                            {   /* Camera */
                                auto& camera            = g_systemConfig.camera;
                                auto& fineSensitivity   = camera.fineSensitivity;
                                auto& coarseSensitivity = camera.coarseSensitivity;

                                ImGui::SeparatorText ("General");
                                ImGui::DragFloat     ("Level damp", &camera.levelDamp,            meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);

                                createHelpMarker     ("Cycle senstivity type to use the updated values");
                                ImGui::SeparatorText ("Fine sensitivity");
                                ImGui::PushID        (0);
                                ImGui::DragFloat     ("Cursor",     &fineSensitivity.cursor,      meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Scroll",     &fineSensitivity.scroll,      meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Movement",   &fineSensitivity.movement,    meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Fov",        &fineSensitivity.fov,         meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Delta damp", &fineSensitivity.deltaDamp,   meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::PopID();

                                ImGui::SeparatorText ("Coarse sensitivity");
                                ImGui::PushID        (1);
                                ImGui::DragFloat     ("Cursor",     &coarseSensitivity.cursor,    meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Scroll",     &coarseSensitivity.scroll,    meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Movement",   &coarseSensitivity.movement,  meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Fov",        &coarseSensitivity.fov,       meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::DragFloat     ("Delta damp", &coarseSensitivity.deltaDamp, meta.dragSpeed,
                                                      0.0f,         FLT_MAX,                      meta.formatSpecifier,
                                                      ImGuiSliderFlags_AlwaysClamp);
                                ImGui::PopID();
                                break;
                            }
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::PopStyleVar();
                ImGui::End();
            }

            ~SYConfigView (void) {
                delete m_configViewInfo.resource.logObj;
            }
    };
}   // namespace SandBox