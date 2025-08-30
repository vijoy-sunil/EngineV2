#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNType.h"

namespace SandBox {
    struct LabelInfo {
        const char* label;
        const char* icon;
    };
    /* Note that, we are using ordererd map since this is how it will be displayed in gui */
    auto g_componentTypeToLabelInfoMap = std::map <Scene::ComponentType, LabelInfo> {
        { 0, {"Meta",             ICON_FA_THUMBTACK        }},
        { 1, {"Mesh",             ICON_FA_CUBE             }},
        { 2, {"Light",            ICON_FA_LIGHTBULB        }},
        { 3, {"Camera",           ICON_FA_CAMERA           }},
        { 4, {"Transform",        ICON_FA_ANCHOR           }},
        { 5, {"TextureIdxOffset", ICON_FA_RIGHT_LEFT       }},
        { 6, {"Color",            ICON_FA_PAINT_ROLLER     }},
        { 7, {"Render",           ICON_FA_FIRE_FLAME_CURVED}},
        { 8, {"StdNoAlphaTag",    ICON_FA_TAGS             }},
        { 9, {"StdAlphaTag",      ICON_FA_TAGS             }},
        {10, {"WireTag",          ICON_FA_TAGS             }},
        {11, {"SkyBoxTag",        ICON_FA_TAGS             }}
    };

    void createHelpMarker (const char* text,
                           const ImVec4 iconColor = ImVec4 (1.0f, 1.0f, 0.0f, 1.0f)) {

        ImGui::PushStyleColor (ImGuiCol_Text, iconColor);
        ImGui::Text           (ICON_FA_TRIANGLE_EXCLAMATION);
        ImGui::PopStyleColor();

        if (ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos (ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted (text);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}   // namespace SandBox