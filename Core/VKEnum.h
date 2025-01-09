#pragma once

namespace Core {
    typedef enum {
        LAYER_LOG                   = 0,
        LAYER_INSTANCE              = 1,
        LAYER_WINDOW                = 2,
        LAYER_SURFACE               = 3,
        LAYER_PHY_DEVICE            = 4,
        LAYER_LOG_DEVICE            = 5,
    } e_layerType;
}   // namespace Core