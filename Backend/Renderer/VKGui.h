#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKInstance.h"
#include "VKWindow.h"
#include "VKPhyDevice.h"
#include "VKLogDevice.h"
#include "VKSwapChain.h"
#include "VKRenderPass.h"
#include "VKDescriptorPool.h"

#define ENABLE_IMGUI_DEMO           (false)
#define ENABLE_IMPLOT_DEMO          (false)

namespace Renderer {
    class VKGui: public Collection::CNTypeInstanceBase {
        private:
            struct GuiInfo {
                struct Meta {
                    const char* iniSaveFilePath;
                    const char* fontFilePath;
                    const char* iconFilePath;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKInstance* instanceObj;
                    VKWindow* windowObj;
                    VKPhyDevice* phyDeviceObj;
                    VKLogDevice* logDeviceObj;
                    VKSwapChain* swapChainObj;
                    VKRenderPass* renderPassObj;
                    VKDescriptorPool* descPoolObj;
                    std::vector <VkDescriptorSet> descriptorSets;
                } resource;
            } m_guiInfo;

            static Log::LGImpl* m_guiLogObj;
            static void debugCallback (const VkResult result) {
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_guiLogObj) << "[?] Gui"
                                            << " "
                                            << "[" << string_VkResult (result) << "]"
                                            << std::endl;
                    throw std::runtime_error ("[?] Gui");
                }
            }

            void createGui (void) {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImPlot::CreateContext();

                auto& meta        = m_guiInfo.meta;
                auto& resource    = m_guiInfo.resource;
                auto& imguiStyle  = ImGui::GetStyle();
                auto& imguiIO     = ImGui::GetIO();
                auto& implotStyle = ImPlot::GetStyle();

                /* Imgui config */
                {   /* Font */
                    imguiStyle.FontSizeBase  = 0.0f;
                    imguiStyle.FontScaleMain = 1.0f;
                    imguiStyle.FontScaleDpi  = 1.0f;
                }
                {   /* Alpha */
                    imguiStyle.Alpha         = 1.0f;
                    imguiStyle.DisabledAlpha = 0.5f;
                }
                {   /* Padding */
                    imguiStyle.WindowPadding            = ImVec2 (4.0f, 4.0f);
                    imguiStyle.WindowBorderHoverPadding = 4.0f;
                    imguiStyle.FramePadding             = ImVec2 (4.0f, 4.0f);
                    imguiStyle.CellPadding              = ImVec2 (0.0f, 0.0f);
                    imguiStyle.TouchExtraPadding        = ImVec2 (0.0f, 0.0f);
                    imguiStyle.SeparatorTextPadding     = ImVec2 (0.0f, 4.0f);
                    imguiStyle.DisplayWindowPadding     = ImVec2 (0.0f, 0.0f);
                    imguiStyle.DisplaySafeAreaPadding   = ImVec2 (0.0f, 0.0f);
                }
                {   /* Rounding */
                    imguiStyle.WindowRounding    = 2.0f;
                    imguiStyle.ChildRounding     = 0.0f;
                    imguiStyle.PopupRounding     = 2.0f;
                    imguiStyle.FrameRounding     = 2.0f;
                    imguiStyle.ScrollbarRounding = 2.0f;
                    imguiStyle.GrabRounding      = 0.0f;
                    imguiStyle.TabRounding       = 2.0f;
                    imguiStyle.TreeLinesRounding = 4.0f;
                }
                {   /* Size */
                    imguiStyle.WindowBorderSize                 = 0.0f;
                    imguiStyle.WindowMinSize                    = ImVec2 (32.0f, 32.0f);
                    imguiStyle.ChildBorderSize                  = 0.0f;
                    imguiStyle.PopupBorderSize                  = 0.0f;
                    imguiStyle.FrameBorderSize                  = 0.0f;
                    imguiStyle.ScrollbarSize                    = 12.0f;
                    imguiStyle.GrabMinSize                      = 0.0f;
                    imguiStyle.LogSliderDeadzone                = 0.0f;
                    imguiStyle.ImageBorderSize                  = 0.0f;
                    imguiStyle.TabBorderSize                    = 0.0f;
                    imguiStyle.TabCloseButtonMinWidthSelected   = 0.0f;
                    imguiStyle.TabCloseButtonMinWidthUnselected = 0.0f;
                    imguiStyle.TabBarBorderSize                 = 0.0f;
                    imguiStyle.TabBarOverlineSize               = 0.0f;
                    imguiStyle.TreeLinesSize                    = 1.0f;
                    imguiStyle.SeparatorTextBorderSize          = 1.0f;
                    imguiStyle.DockingSeparatorSize             = 1.0f;
                    imguiStyle.MouseCursorScale                 = 1.0f;
                }
                {   /* Alignment */
                    imguiStyle.WindowTitleAlign            = ImVec2 (0.0f, 0.5f);
                    imguiStyle.WindowMenuButtonPosition    = ImGuiDir_Left;
                    imguiStyle.TableAngledHeadersAngle     = 0.0f;
                    imguiStyle.TableAngledHeadersTextAlign = ImVec2 (0.0f, 0.0f);
                    imguiStyle.ColorButtonPosition         = ImGuiDir_Right;
                    imguiStyle.ButtonTextAlign             = ImVec2 (0.5f, 0.5f);
                    imguiStyle.SelectableTextAlign         = ImVec2 (0.0f, 0.0f);
                    imguiStyle.SeparatorTextAlign          = ImVec2 (0.0f, 0.5f);
                }
                {   /* Spacing */
                    imguiStyle.ItemSpacing       = ImVec2 (4.0f, 4.0f);
                    imguiStyle.ItemInnerSpacing  = ImVec2 (4.0f, 4.0f);
                    imguiStyle.IndentSpacing     = 24.0f;
                    imguiStyle.ColumnsMinSpacing =  0.0f;
                }
                {   /* Misc */
                    imguiStyle.TreeLinesFlags             = ImGuiTreeNodeFlags_DrawLinesToNodes;
                    imguiStyle.AntiAliasedLines           = true;
                    imguiStyle.AntiAliasedLinesUseTex     = true;
                    imguiStyle.AntiAliasedFill            = true;
                    imguiStyle.CurveTessellationTol       = 1.25f;
                    imguiStyle.CircleTessellationMaxError = 0.30f;
                    imguiStyle.HoverStationaryDelay       = 0.15f;
                    imguiStyle.HoverDelayShort            = 0.15f;
                    imguiStyle.HoverDelayNormal           = 0.40f;
                    imguiStyle.HoverFlagsForTooltipMouse  = ImGuiHoveredFlags_Stationary  |
                                                            ImGuiHoveredFlags_DelayNormal |
                                                            ImGuiHoveredFlags_NoSharedDelay;
                    imguiStyle.HoverFlagsForTooltipNav    = ImGuiHoveredFlags_None;
                }
                {   /* Color */
                    auto& colors                               = imguiStyle.Colors;
                    ImVec4 unusedColor                         = ImVec4 (1.0f, 0.0f, 1.0f, 1.0f);
                    /* Border */
                    colors[ImGuiCol_Border]                    = ImVec4 (0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImGuiCol_BorderShadow]              = ImVec4 (0.0f, 0.0f, 0.0f, 0.0f);
                    /* Window */
                    colors[ImGuiCol_WindowBg]                  = ImVec4 (0.11f, 0.11f, 0.11f, 1.0f);
                    colors[ImGuiCol_ChildBg]                   = ImVec4 (0.17f, 0.17f, 0.17f, 1.0f);
                    colors[ImGuiCol_NavWindowingHighlight]     = colors[ImGuiCol_Border];
                    colors[ImGuiCol_NavWindowingDimBg]         = unusedColor;
                    colors[ImGuiCol_ModalWindowDimBg]          = unusedColor;
                    /* Frame */
                    colors[ImGuiCol_FrameBg]                   = ImVec4 (0.35f, 0.35f, 0.35f, 0.3f);
                    colors[ImGuiCol_FrameBgHovered]            = ImVec4 (0.35f, 0.35f, 0.35f, 0.8f);
                    colors[ImGuiCol_FrameBgActive]             = ImVec4 (0.35f, 0.35f, 0.35f, 1.0f);
                    /* Header */
                    colors[ImGuiCol_Header]                    = colors[ImGuiCol_FrameBg];
                    colors[ImGuiCol_HeaderHovered]             = colors[ImGuiCol_FrameBgHovered];
                    colors[ImGuiCol_HeaderActive]              = colors[ImGuiCol_FrameBgActive];
                    /* Title */
                    colors[ImGuiCol_TitleBg]                   = colors[ImGuiCol_FrameBg];
                    colors[ImGuiCol_TitleBgCollapsed]          = colors[ImGuiCol_FrameBg];
                    colors[ImGuiCol_TitleBgActive]             = colors[ImGuiCol_FrameBgActive];
                    /* Button */
                    colors[ImGuiCol_Button]                    = colors[ImGuiCol_FrameBg];
                    colors[ImGuiCol_ButtonHovered]             = colors[ImGuiCol_FrameBgHovered];
                    colors[ImGuiCol_ButtonActive]              = colors[ImGuiCol_FrameBgActive];
                    /* Tab */
                    colors[ImGuiCol_Tab]                       = ImVec4 (0.45f, 0.45f, 0.45f, 0.3f);
                    colors[ImGuiCol_TabHovered]                = ImVec4 (0.45f, 0.45f, 0.45f, 0.8f);
                    colors[ImGuiCol_TabSelected]               = ImVec4 (0.45f, 0.45f, 0.45f, 1.0f);
                    colors[ImGuiCol_TabDimmed]                 = colors[ImGuiCol_Tab];
                    colors[ImGuiCol_TabDimmedSelected]         = colors[ImGuiCol_TabSelected];
                    colors[ImGuiCol_TabSelectedOverline]       = unusedColor;
                    colors[ImGuiCol_TabDimmedSelectedOverline] = unusedColor;
                    /* Scroll bar */
                    colors[ImGuiCol_ScrollbarGrab]             = ImVec4 (0.0f, 1.0f, 0.0f, 0.3f);
                    colors[ImGuiCol_ScrollbarGrabHovered]      = ImVec4 (0.0f, 1.0f, 0.0f, 0.8f);
                    colors[ImGuiCol_ScrollbarGrabActive]       = ImVec4 (0.0f, 1.0f, 0.0f, 1.0f);
                    colors[ImGuiCol_ScrollbarBg]               = ImVec4 (0.0f, 0.0f, 0.0f, 1.0f);
                    /* Resize grip */
                    colors[ImGuiCol_ResizeGrip]                = colors[ImGuiCol_ScrollbarGrab];
                    colors[ImGuiCol_ResizeGripHovered]         = colors[ImGuiCol_ScrollbarGrabHovered];
                    colors[ImGuiCol_ResizeGripActive]          = colors[ImGuiCol_ScrollbarGrabActive];
                    /* Check mark */
                    colors[ImGuiCol_CheckMark]                 = colors[ImGuiCol_ScrollbarGrabActive];
                    /* Slider grab */
                    colors[ImGuiCol_SliderGrab]                = unusedColor;
                    colors[ImGuiCol_SliderGrabActive]          = unusedColor;
                    /* Docking */
                    colors[ImGuiCol_DockingEmptyBg]            = colors[ImGuiCol_ScrollbarBg];
                    colors[ImGuiCol_DockingPreview]            = colors[ImGuiCol_ScrollbarGrab];
                    /* Drag-drop */
                    colors[ImGuiCol_DragDropTarget]            = colors[ImGuiCol_ScrollbarGrab];
                    /* Pop up */
                    colors[ImGuiCol_PopupBg]                   = colors[ImGuiCol_ScrollbarBg];
                    /* Text */
                    colors[ImGuiCol_Text]                      = ImVec4 (1.0f, 1.0f, 1.0f, 1.0f);
                    colors[ImGuiCol_TextDisabled]              = colors[ImGuiCol_Text];
                    colors[ImGuiCol_TextSelectedBg]            = colors[ImGuiCol_ScrollbarGrab];
                    colors[ImGuiCol_InputTextCursor]           = colors[ImGuiCol_ScrollbarGrabActive];
                    colors[ImGuiCol_TextLink]                  = unusedColor;
                    /* Separator */
                    colors[ImGuiCol_Separator]                 = colors[ImGuiCol_ScrollbarGrab];
                    colors[ImGuiCol_SeparatorHovered]          = colors[ImGuiCol_ScrollbarGrab];
                    colors[ImGuiCol_SeparatorActive]           = colors[ImGuiCol_ScrollbarGrab];
                    /* Tree */
                    colors[ImGuiCol_TreeLines]                 = colors[ImGuiCol_ScrollbarGrab];
                    /* Table */
                    colors[ImGuiCol_TableHeaderBg]             = unusedColor;
                    colors[ImGuiCol_TableBorderStrong]         = unusedColor;
                    colors[ImGuiCol_TableBorderLight]          = unusedColor;
                    colors[ImGuiCol_TableRowBg]                = unusedColor;
                    colors[ImGuiCol_TableRowBgAlt]             = unusedColor;
                    /* Plot (see implot) */
                    colors[ImGuiCol_PlotLines]                 = unusedColor;
                    colors[ImGuiCol_PlotLinesHovered]          = unusedColor;
                    colors[ImGuiCol_PlotHistogram]             = unusedColor;
                    colors[ImGuiCol_PlotHistogramHovered]      = unusedColor;
                    /* Other */
                    colors[ImGuiCol_MenuBarBg]                 = unusedColor;
                    colors[ImGuiCol_NavCursor]                 = unusedColor;
                }
                {   /* Font & icon */
                    float fontSize                            = 12.0f;
                    float iconSize                            = fontSize;

                    imguiIO.ConfigFlags                      |= ImGuiConfigFlags_DockingEnable;
                    imguiIO.IniFilename                       = meta.iniSaveFilePath;
                    imguiIO.ConfigWindowsMoveFromTitleBarOnly = true;
                    imguiIO.ConfigDragClickToInputText        = true;
                    /* Note that, if no fonts are loaded, imgui will use the default font */
                    imguiIO.Fonts->AddFontFromFileTTF (meta.fontFilePath,
                                                       fontSize,
                                                       nullptr,
                                                       imguiIO.Fonts->GetGlyphRangesDefault());
                    /* Merge icons into font */
                    ImFontConfig fontConfig                   = {};
                    fontConfig.MergeMode                      = true;
                    fontConfig.PixelSnapH                     = true;
                    fontConfig.GlyphMinAdvanceX               = iconSize;
                    static const ImWchar glyphRanges[]        = {
                        ICON_MIN_FA,
                        ICON_MAX_16_FA,
                        0
                    };
                    imguiIO.Fonts->AddFontFromFileTTF (meta.iconFilePath,
                                                       iconSize,
                                                       &fontConfig,
                                                       glyphRanges);
                }
                {   /* Backend */
                    ImGui_ImplGlfw_InitForVulkan (resource.windowObj->getWindow(), true);

                    ImGui_ImplVulkan_InitInfo initInfo = {};
                    initInfo.ApiVersion                = VK_MAKE_API_VERSION (0, 1, 3, 0);
                    initInfo.Instance                  = *resource.instanceObj->getInstance();
                    initInfo.PhysicalDevice            = *resource.phyDeviceObj->getPhyDevice();
                    initInfo.QueueFamily               = resource.phyDeviceObj->getGraphicsQueueFamilyIdx();
                    initInfo.Device                    = *resource.logDeviceObj->getLogDevice();
                    initInfo.Queue                     = *resource.logDeviceObj->getGraphicsQueue();
                    initInfo.MinImageCount             = resource.swapChainObj->getSwapChainMinImages();
                    initInfo.ImageCount                = resource.swapChainObj->getSwapChainImagesCount();
                    initInfo.MSAASamples               = VK_SAMPLE_COUNT_1_BIT;
                    initInfo.RenderPass                = *resource.renderPassObj->getRenderPass();
                    initInfo.DescriptorPool            = *resource.descPoolObj->getDescriptorPool();
                    initInfo.CheckVkResultFn           = debugCallback;
                    ImGui_ImplVulkan_Init (&initInfo);
                }

                /* Implot config */
                {   /* Alpha */
                    implotStyle.FillAlpha  = 0.8f;
                    implotStyle.MinorAlpha = 0.2f;
                }
                {   /* Padding */
                    implotStyle.DigitalBitGap      = 0.0f;
                    implotStyle.PlotPadding        = ImVec2 (0.0f, 0.0f);
                    implotStyle.LabelPadding       = ImVec2 (0.0f, 0.0f);
                    implotStyle.LegendPadding      = ImVec2 (4.0f, 4.0f);
                    implotStyle.LegendInnerPadding = ImVec2 (4.0f, 4.0f);
                    implotStyle.MousePosPadding    = ImVec2 (0.0f, 0.0f);
                    implotStyle.AnnotationPadding  = ImVec2 (0.0f, 0.0f);
                    implotStyle.FitPadding         = ImVec2 (0.0f, 0.0f);
                }
                {   /* Size */
                    implotStyle.LineWeight       = 0.0f;
                    implotStyle.MarkerSize       = 0.0f;
                    implotStyle.MarkerWeight     = 0.0f;
                    implotStyle.ErrorBarSize     = 0.0f;
                    implotStyle.ErrorBarWeight   = 0.0f;
                    implotStyle.DigitalBitHeight = 0.0f;
                    implotStyle.PlotBorderSize   = 0.0f;
                    implotStyle.MajorTickLen     = ImVec2 (  0.0f,   0.0f);
                    implotStyle.MinorTickLen     = ImVec2 (  0.0f,   0.0f);
                    implotStyle.MajorTickSize    = ImVec2 (  0.0f,   0.0f);
                    implotStyle.MinorTickSize    = ImVec2 (  0.0f,   0.0f);
                    implotStyle.MajorGridSize    = ImVec2 (  1.0f,   1.0f);
                    implotStyle.MinorGridSize    = ImVec2 (  1.0f,   1.0f);
                    implotStyle.PlotDefaultSize  = ImVec2 (200.0f, 200.0f);
                    implotStyle.PlotMinSize      = ImVec2 (200.0f,  60.0f);
                }
                {   /* Spacing */
                    implotStyle.LegendSpacing = ImVec2 (0.0f, 0.0f);
                }
                {   /* Misc */
                    implotStyle.Marker         = ImPlotMarker_None;
                    implotStyle.UseLocalTime   = false;
                    implotStyle.Use24HourClock = false;
                    implotStyle.UseISO8601     = false;
                }
                {   /* Color */
                    auto& colors                    = implotStyle.Colors;
                    ImVec4 unusedColor              = ImVec4 (1.0f, 0.0f, 1.0f, 1.0f);
                    /* Frame */
                    colors[ImPlotCol_FrameBg]       = unusedColor;
                    /* Plot */
                    colors[ImPlotCol_Line]          = unusedColor;
                    colors[ImPlotCol_Fill]          = ImVec4 (0.0f, 1.0f, 0.0f, 1.0f);
                    colors[ImPlotCol_PlotBg]        = ImVec4 (0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImPlotCol_PlotBorder]    = unusedColor;
                    /* Legend */
                    colors[ImPlotCol_LegendBg]      = ImVec4 (0.0f, 0.0f, 0.0f, 1.0f);
                    colors[ImPlotCol_LegendBorder]  = ImVec4 (0.0f, 0.0f, 0.0f, 0.0f);
                    colors[ImPlotCol_LegendText]    = ImVec4 (1.0f, 1.0f, 1.0f, 1.0f);
                    /* Text */
                    colors[ImPlotCol_TitleText]     = unusedColor;
                    colors[ImPlotCol_InlayText]     = unusedColor;
                    /* Marker */
                    colors[ImPlotCol_MarkerOutline] = unusedColor;
                    colors[ImPlotCol_MarkerFill]    = unusedColor;
                    /* Error bar */
                    colors[ImPlotCol_ErrorBar]      = unusedColor;
                    /* Axis */
                    colors[ImPlotCol_AxisBg]        = unusedColor;
                    colors[ImPlotCol_AxisBgHovered] = unusedColor;
                    colors[ImPlotCol_AxisBgActive]  = unusedColor;
                    colors[ImPlotCol_AxisTick]      = unusedColor;
                    colors[ImPlotCol_AxisText]      = unusedColor;
                    colors[ImPlotCol_AxisGrid]      = ImVec4 (1.0f, 1.0f, 0.0f, 1.0f);
                    /* Other */
                    colors[ImPlotCol_Selection]     = unusedColor;
                    colors[ImPlotCol_Crosshairs]    = unusedColor;
                }
            }

            void destroyGui (void) {
                for (auto const& descriptorSet: m_guiInfo.resource.descriptorSets)
                    ImGui_ImplVulkan_RemoveTexture (descriptorSet);

                ImPlot::DestroyContext();
                ImGui_ImplVulkan_Shutdown();
                ImGui_ImplGlfw_Shutdown();
                ImGui::DestroyContext();
            }

        public:
            VKGui (Log::LGImpl*      logObj,
                   VKInstance*       instanceObj,
                   VKWindow*         windowObj,
                   VKPhyDevice*      phyDeviceObj,
                   VKLogDevice*      logDeviceObj,
                   VKSwapChain*      swapChainObj,
                   VKRenderPass*     renderPassObj,
                   VKDescriptorPool* descPoolObj) {

                m_guiInfo = {};

                if (logObj == nullptr) {
                    m_guiInfo.resource.logObj     = new Log::LGImpl();
                    m_guiInfo.state.logObjCreated = true;

                    m_guiInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_guiInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                            << std::endl;
                }
                else {
                    m_guiInfo.resource.logObj     = logObj;
                    m_guiInfo.state.logObjCreated = false;
                }

                m_guiLogObj = new Log::LGImpl();
                m_guiLogObj->initLogInfo     ("Build/Log/Renderer",    "Gui");
                m_guiLogObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_NONE);
                m_guiLogObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_NONE);
                m_guiLogObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);

                if (instanceObj  == nullptr || windowObj     == nullptr ||
                    phyDeviceObj == nullptr || logDeviceObj  == nullptr ||
                    swapChainObj == nullptr || renderPassObj == nullptr || descPoolObj == nullptr) {

                    LOG_ERROR (m_guiInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                          << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_guiInfo.resource.instanceObj    = instanceObj;
                m_guiInfo.resource.windowObj      = windowObj;
                m_guiInfo.resource.phyDeviceObj   = phyDeviceObj;
                m_guiInfo.resource.logDeviceObj   = logDeviceObj;
                m_guiInfo.resource.swapChainObj   = swapChainObj;
                m_guiInfo.resource.renderPassObj  = renderPassObj;
                m_guiInfo.resource.descPoolObj    = descPoolObj;
            }

            void initGuiInfo (const char* iniSaveFilePath,
                              const char* fontFilePath,
                              const char* iconFilePath) {

                m_guiInfo.meta.iniSaveFilePath    = iniSaveFilePath;
                m_guiInfo.meta.fontFilePath       = fontFilePath;
                m_guiInfo.meta.iconFilePath       = iconFilePath;
                m_guiInfo.resource.descriptorSets = {};
            }

            ImTextureID addTexture (const VkImageView imageView,
                                    const VkSampler sampler,
                                    const VkImageLayout imageLayout) {

                auto descriptorSet = ImGui_ImplVulkan_AddTexture (sampler, imageView, imageLayout);
                m_guiInfo.resource.descriptorSets.push_back      (descriptorSet);
                return reinterpret_cast <ImTextureID>            (descriptorSet);
            }

            void toggleCursorInput (const bool val) {
                auto& imguiIO = ImGui::GetIO();
                if (val)        imguiIO.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
                else            imguiIO.ConfigFlags |=  ImGuiConfigFlags_NoMouse;
            }

            void beginFrame (void) {
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                /* Create an explicit dock node covering the main view port */
                ImGui::DockSpaceOverViewport (0,
                                              ImGui::GetMainViewport(),
                                              ImGuiDockNodeFlags_PassthruCentralNode);
#if ENABLE_IMGUI_DEMO
                ImGui::ShowDemoWindow();
#endif  // ENABLE_IMGUI_DEMO
#if ENABLE_IMPLOT_DEMO
                ImPlot::ShowDemoWindow();
#endif  // ENABLE_IMPLOT_DEMO
            }

            void endFrame (void) {
                ImGui::Render();
            }

            void onAttach (void) override {
                createGui();
            }

            void onDetach (void) override {
                destroyGui();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKGui (void) {
                if (m_guiInfo.state.logObjCreated)
                    delete m_guiInfo.resource.logObj;
                delete m_guiLogObj;
            }
    };
    Log::LGImpl* VKGui::m_guiLogObj = nullptr;
}   // namespace Renderer