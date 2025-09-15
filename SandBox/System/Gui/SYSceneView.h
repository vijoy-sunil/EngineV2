#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKImage.h"
#include "../../Backend/Renderer/VKSampler.h"
#include "../../Backend/Renderer/VKGui.h"

namespace SandBox {
    class SYSceneView: public Scene::SNSystemBase {
        private:
            struct ScrollingBuffer {
                std::vector <ImVec2> m_buffer = {};
                /* Higher fps and plot history will determine the max buffer size, 60fps * 10s = 600-700 frames */
                size_t m_maxSize              = 1000;
                size_t m_offset               = 0;
                float m_average               = 0.0f;

                void addPoint (const float x, const float y) {
                    if (m_buffer.size() < m_maxSize) {
                        m_buffer.push_back (ImVec2 (x, y));
                        m_average = ((m_average * (m_buffer.size() - 1)) + y) / m_buffer.size();
                    }
                    else {
                        float previousY    = m_buffer[m_offset].y;
                        m_buffer[m_offset] = ImVec2 (x, y);
                        m_offset           = (m_offset + 1) % m_maxSize;
                        m_average         += (y - previousY) / m_maxSize;
                    }
                }
            };

            struct SceneViewInfo {
                struct Meta {
                    float elapsedTime;
                    float plotHistory;
                    float minFpsPlotAxis;
                    float maxFpsPlotAxis;

                    ScrollingBuffer fpsBuffer;
                    ImTextureID textureId;
                } meta;

                struct Style {
                    float plotWindowAlpha;
                    ImVec2 sceneWindowPadding;
                    ImVec2 plotWindowPadding;
                    ImVec2 plotSize;
                } style;

                struct Flag {
                    ImGuiWindowFlags sceneWindowFlags;
                    ImGuiWindowFlags plotWindowFlags;
                    ImPlotFlags plotFlags;
                    ImPlotAxisFlags plotAxisFlags;
                } flag;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_sceneViewInfo;

        public:
            SYSceneView (void) {
                m_sceneViewInfo = {};

                auto& logObj = m_sceneViewInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initSceneViewInfo (Collection::CNImpl* collectionObj) {
                auto& meta               = m_sceneViewInfo.meta;
                auto& style              = m_sceneViewInfo.style;
                auto& flag               = m_sceneViewInfo.flag;

                meta.elapsedTime         =  0.0f;
                meta.plotHistory         = 10.0f;
                meta.minFpsPlotAxis      = 30.0f;
                meta.maxFpsPlotAxis      = 90.0f;
                meta.fpsBuffer           = {};

                style.plotWindowAlpha    = 0.5f;
                style.sceneWindowPadding = ImVec2 (  0.0f,  0.0f);
                style.plotWindowPadding  = ImVec2 (  0.0f,  0.0f);
                style.plotSize           = ImVec2 (200.0f, 60.0f);

                flag.sceneWindowFlags    = ImGuiWindowFlags_NoScrollbar       |
                                           ImGuiWindowFlags_NoScrollWithMouse;
                flag.plotWindowFlags     = ImGuiWindowFlags_NoTitleBar        |
                                           ImGuiWindowFlags_NoResize          |
                                           ImGuiWindowFlags_NoMove            |
                                           ImGuiWindowFlags_NoScrollbar       |
                                           ImGuiWindowFlags_NoScrollWithMouse |
                                           ImGuiWindowFlags_NoCollapse        |
                                           ImGuiWindowFlags_NoMouseInputs     |
                                           ImGuiWindowFlags_NoNavInputs       |
                                           ImGuiWindowFlags_NoNavFocus        |
                                           ImGuiWindowFlags_NoDocking;
                flag.plotFlags           = ImPlotFlags_NoTitle                |
                                           ImPlotFlags_NoMouseText            |
                                           ImPlotFlags_NoInputs               |
                                           ImPlotFlags_NoMenus                |
                                           ImPlotFlags_NoBoxSelect            |
                                           ImPlotFlags_NoFrame;
                flag.plotAxisFlags       = ImPlotAxisFlags_NoLabel            |
                                           ImPlotAxisFlags_NoTickMarks        |
                                           ImPlotAxisFlags_NoTickLabels       |
                                           ImPlotAxisFlags_NoMenus            |
                                           ImPlotAxisFlags_NoSideSwitch       |
                                           ImPlotAxisFlags_NoHighlight        |
                                           ImPlotAxisFlags_LockMin            |
                                           ImPlotAxisFlags_LockMax;

                if (collectionObj == nullptr) {
                    LOG_ERROR (m_sceneViewInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                auto imageObj            = collectionObj->getCollectionTypeInstance <Renderer::VKImage>   (
                    "F_LIGHT_COLOR"
                );
                auto samplerObj          = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> (
                    "F_LIGHT_GBUFFER"
                );
                auto guiObj              = collectionObj->getCollectionTypeInstance <Renderer::VKGui>     (
                    "DRAW_OPS"
                );
                meta.textureId           = guiObj->addTexture (
                    *imageObj->getImageView(),
                    *samplerObj->getSampler(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );
            }

            void update (const float frameDelta) {
                auto& meta        = m_sceneViewInfo.meta;
                auto& style       = m_sceneViewInfo.style;
                auto& flag        = m_sceneViewInfo.flag;

                /* Add (x,y) point to buffer */
                meta.elapsedTime += frameDelta;
                float fps         = frameDelta == 0.0f ? 0.0f: (1.0f / frameDelta);
                meta.fpsBuffer.addPoint (meta.elapsedTime, fps);

                /* Construct legend label */
                char label[11];
                std::snprintf (label, sizeof (label), "FPS: %0.2f", meta.fpsBuffer.m_average);

                /* Set plot window position */
                float titleBarHeight      = ImGui::GetTextLineHeight() + (2.0f * ImGui::GetStyle().FramePadding.y);
                ImVec2 plotWindowPosition = ImVec2 (ImGui::GetStyle().WindowPadding.x,
                                                    ImGui::GetStyle().WindowPadding.y + titleBarHeight);

                ImGui::PushStyleVar (ImGuiStyleVar_WindowPadding, style.sceneWindowPadding);
                if (ImGui::Begin    (ICON_FA_EYE " Scene", nullptr, flag.sceneWindowFlags)) {
                    /* Compute image size */
                    ImVec2 contentRegionSize = ImGui::GetContentRegionAvail();
                    ImGui::Image                    (meta.textureId, contentRegionSize);

                    ImGui::SetNextWindowPos         (plotWindowPosition);
                    ImGui::SetNextWindowBgAlpha     (style.plotWindowAlpha);

                    ImGui::PushStyleVar             (ImGuiStyleVar_WindowPadding, style.plotWindowPadding);
                    if (ImGui::Begin                ("Plot", nullptr, flag.plotWindowFlags)) {
                        if (ImPlot::BeginPlot       ("FPS", style.plotSize, flag.plotFlags)) {

                            ImPlot::SetupAxes       (nullptr, nullptr, flag.plotAxisFlags, flag.plotAxisFlags);
                            ImPlot::SetupAxisLimits (ImAxis_X1,
                                                     meta.elapsedTime - meta.plotHistory,
                                                     meta.elapsedTime,
                                                     ImGuiCond_Always);
                            ImPlot::SetupAxisLimits (ImAxis_Y1,
                                                     meta.minFpsPlotAxis,
                                                     meta.maxFpsPlotAxis,
                                                     ImGuiCond_Always);

                            ImPlot::PlotShaded      (label,
                                                     &meta.fpsBuffer.m_buffer[0].x,
                                                     &meta.fpsBuffer.m_buffer[0].y,
                                                     static_cast <int> (meta.fpsBuffer.m_buffer.size()),
                                                     -INFINITY,
                                                     ImPlotShadedFlags_None,
                                                     static_cast <int> (meta.fpsBuffer.m_offset),
                                                     2 * sizeof (float));
                            ImPlot::EndPlot();
                        }
                    }
                    ImGui::PopStyleVar();
                    ImGui::End();
                }
                ImGui::PopStyleVar();
                ImGui::End();
            }

            ~SYSceneView (void) {
                delete m_sceneViewInfo.resource.logObj;
            }
    };
}   // namespace SandBox