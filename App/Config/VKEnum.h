#pragma once

namespace Renderer {
    typedef enum {
        LAYER_LOG                       = 0,

        LAYER_MODEL                     = 1,
        LAYER_LIGHT                     = 2,
        LAYER_CAMERA                    = 3,

        LAYER_INSTANCE                  = 4,
        LAYER_WINDOW                    = 5,
        LAYER_SURFACE                   = 6,
        LAYER_PHY_DEVICE                = 7,
        LAYER_LOG_DEVICE                = 8,
        LAYER_SWAP_CHAIN                = 9,

        LAYER_STAGING_BUFFER            = 10,
        LAYER_VERTEX_BUFFER             = 11,
        LAYER_INDEX_BUFFER              = 12,
        LAYER_UNIFORM_BUFFER            = 13,
        LAYER_STORAGE_BUFFER            = 14,

        LAYER_COLOR_IMAGE               = 15,
        LAYER_DEPTH_IMAGE               = 16,
        LAYER_SWAP_CHAIN_IMAGE          = 17,
        LAYER_TEXTURE_IMAGE             = 18,

        LAYER_SAMPLER                   = 19,

        LAYER_RENDER_PASS               = 20,
        LAYER_FRAME_BUFFER              = 21,
        LAYER_PIPELINE                  = 22,
        LAYER_DESCRIPTOR_POOL           = 23,
        LAYER_DESCRIPTOR_SET_DYNAMIC    = 24,
        LAYER_DESCRIPTOR_SET_STATIC     = 25,

        LAYER_SYNC_PRIMITIVE            = 26,

        LAYER_CMD_POOL                  = 27,
        LAYER_CMD_BUFFER                = 28
    } e_layerType;
}   // namespace Renderer