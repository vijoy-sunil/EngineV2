#pragma once

namespace SandBox {
    struct SystemConfig {
        struct Shadow {
            float minShadowFactor           =  0.0f;
            float depthBiasConstantFactor   =  4.0f;
            float depthBiasSlopeFactor      =  1.5f;
            float minDepthBias              = 0.01f;
            float maxDepthBias              =  0.2f;
        } shadow;

        struct Camera {
            float levelDamp                 =  2.5f;
            struct FineSensitivity {
                float cursor                = 0.03f;
                float scroll                = 0.25f;
                float movement              = 1.00f;
                float fov                   = 0.10f;
                float deltaDamp             = 0.00f;
            } fineSensitivity;

            struct CoarseSensitivity {
                float cursor                = 0.08f;
                float scroll                = 0.90f;
                float movement              = 6.50f;
                float fov                   = 0.40f;
                float deltaDamp             = 0.85f;
            } coarseSensitivity;
        } camera;
    } g_systemConfig;
}   // namespace SandBox