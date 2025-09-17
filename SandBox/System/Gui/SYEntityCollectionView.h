#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Scene/SNType.h"
#include "SYGuiHelper.h"
#include "../../SBComponentType.h"

namespace SandBox {
    class SYEntityCollectionView: public Scene::SNSystemBase {
        private:
            struct EntityCollectionViewInfo {
                struct Meta {
                    Scene::Entity selectedEntity;
                    Scene::ComponentType selectedComponentType;
                    std::map <Scene::Entity, std::vector <Scene::Entity>> parentEntityToChildrenMap;
                } meta;

                struct Style {
                    float buttonOutlineRounding;
                    float buttonOutlineThickness;
                    ImVec2 buttonSize;
                    ImVec2 treeNodeSpacing;
                    ImVec4 selectedTreeNodeTextColor;
                    ImVec4 buttonOutlineColor;
                } style;

                struct Flag {
                    ImGuiTreeNodeFlags treeNodeFlags;
                } flag;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_entityCollectionViewInfo;

            bool createEntityTreeNode (const Scene::Entity entity) {
                auto& meta            = m_entityCollectionViewInfo.meta;
                auto& style           = m_entityCollectionViewInfo.style;
                auto& treeNodeFlags   = m_entityCollectionViewInfo.flag.treeNodeFlags;
                auto& sceneObj        = m_entityCollectionViewInfo.resource.sceneObj;
                std::string label     = sceneObj->getComponent <MetaComponent> (entity)->m_id;
                bool treeNodeOpened   = false;
                bool treeNodeSelected = meta.selectedEntity == entity;

                if (treeNodeSelected)              treeNodeFlags |=  ImGuiTreeNodeFlags_Selected;
                else                               treeNodeFlags &= ~ImGuiTreeNodeFlags_Selected;

                ImGui::PushStyleVar                (ImGuiStyleVar_ItemSpacing, style.treeNodeSpacing);
                if (treeNodeSelected)
                    ImGui::PushStyleColor          (ImGuiCol_Text, style.selectedTreeNodeTextColor);

                treeNodeOpened = ImGui::TreeNodeEx (label.c_str(), treeNodeFlags);
                if (treeNodeSelected)
                    ImGui::PopStyleColor();
                ImGui::PopStyleVar();

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                    meta.selectedEntity  = entity;
                    auto entitySignature = sceneObj->getEntitySignature (meta.selectedEntity);

                    if (!entitySignature[meta.selectedComponentType])
                        meta.selectedComponentType = 0;
                }

                return treeNodeOpened;
            }

            void createComponentTypeButtons (const Scene::Entity entity) {
                auto& meta           = m_entityCollectionViewInfo.meta;
                auto& style          = m_entityCollectionViewInfo.style;
                auto entitySignature = m_entityCollectionViewInfo.resource.sceneObj->getEntitySignature (entity);
                size_t loopIdx       = 0;

                for (auto const& [type, info]: g_componentTypeToLabelInfoMap) {
                    bool buttonClicked  = false;
                    bool buttonSelected = meta.selectedComponentType == type;
                    if (!entitySignature[type])
                        continue;

                    /* Note that, we need to manually add spacing between bottom of tree node and top of buttons, since
                     * tree nodes are populated without any item spacing
                    */
                    if (loopIdx == 0)
                        ImGui::SetCursorPos       (ImVec2 (ImGui::GetCursorPos().x,
                                                           ImGui::GetCursorPos().y + ImGui::GetStyle().FramePadding.y));
                    else
                        ImGui::SameLine();

                    ImGui::PushID                 (loopIdx);
                    buttonClicked = ImGui::Button (info.icon, style.buttonSize);
                    /* Outline on selected button */
                    if (buttonSelected) {
                        ImVec2 upperLeftCoord  = ImGui::GetItemRectMin();
                        ImVec2 lowerRightCoord = ImGui::GetItemRectMax();
                        ImGui::GetWindowDrawList()->AddRect (upperLeftCoord,
                                                             lowerRightCoord,
                                                             ImGui::ColorConvertFloat4ToU32 (style.buttonOutlineColor),
                                                             style.buttonOutlineRounding,
                                                             ImDrawFlags_RoundCornersAll,
                                                             style.buttonOutlineThickness);
                    }
                    ImGui::PopID();
                    ImGui::SetItemTooltip ("%s", info.label);

                    if (buttonClicked) {
                        meta.selectedEntity        = entity;
                        meta.selectedComponentType = type;
                    }
                    ++loopIdx;
                }
            }

        public:
            SYEntityCollectionView (void) {
                m_entityCollectionViewInfo = {};

                auto& logObj = m_entityCollectionViewInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initEntityCollectionViewInfo (const Scene::Entity selectedEntity,
                                               const std::map <Scene::Entity, std::vector <Scene::Entity>>
                                               parentEntityToChildrenMap,
                                               Scene::SNImpl* sceneObj) {

                auto& meta                                    = m_entityCollectionViewInfo.meta;
                auto& style                                   = m_entityCollectionViewInfo.style;
                auto& resource                                = m_entityCollectionViewInfo.resource;

                meta.selectedEntity                           = selectedEntity;
                meta.selectedComponentType                    = 0;
                meta.parentEntityToChildrenMap                = parentEntityToChildrenMap;

                style.buttonOutlineRounding                   = ImGui::GetStyle().FrameRounding;
                style.buttonOutlineThickness                  = ImGui::GetStyle().TreeLinesSize;
                style.buttonSize                              = ImVec2 (24.0f, 24.0f);
                style.treeNodeSpacing                         = ImVec2 ( 0.0f,  0.0f);
                style.selectedTreeNodeTextColor               = ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive];
                style.buttonOutlineColor                      = ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive];

                m_entityCollectionViewInfo.flag.treeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                                                ImGuiTreeNodeFlags_OpenOnArrow       |
                                                                ImGuiTreeNodeFlags_FramePadding      |
                                                                ImGuiTreeNodeFlags_SpanFullWidth;

                if (sceneObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.sceneObj                             = sceneObj;
            }

            Scene::Entity getSelectedEntity (void) {
                return m_entityCollectionViewInfo.meta.selectedEntity;
            }

            /* Note that, we return a reference to the selected component type allowing us to modify it externally */
            Scene::ComponentType& getSelectedComponentType (void) {
                return m_entityCollectionViewInfo.meta.selectedComponentType;
            }

            void update (void) {
                if (ImGui::Begin (ICON_FA_SHAPES " Entity collection", nullptr, ImGuiWindowFlags_None)) {
                    for (auto const& [parent, children]: m_entityCollectionViewInfo.meta.parentEntityToChildrenMap) {
                        if (createEntityTreeNode       (parent)) {
                            createComponentTypeButtons (parent);

                            for (auto const& child: children) {
                                if (createEntityTreeNode       (child)) {
                                    createComponentTypeButtons (child);
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                    }
                }
                ImGui::End();
            }

            ~SYEntityCollectionView (void) {
                delete m_entityCollectionViewInfo.resource.logObj;
            }
    };
}   // namespace SandBox