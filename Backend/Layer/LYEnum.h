#pragma once

namespace Layer {
    typedef enum {
        LAYER_POOL_DOES_NOT_EXIST                       = -1,
        LAYER_POOL_ALREADY_EXISTS                       = -2,
        LAYER_DOES_NOT_EXIST                            = -3,
        LAYER_ALREADY_EXISTS                            = -4,
        LAYER_INSTANCE_DOES_NOT_EXIST                   = -5,
        LAYER_INSTANCE_ALREADY_EXISTS                   = -6,
        LAYER_INSTANCE_RUN_DISABLED                     = -7,
        LAYER_INSTANCE_MISSING_BINDING                  = -8,
        VALID_REQUEST                                   =  0,
        PENDING_REQUEST                                 =  1,
    } e_layerCode;

    const char* getLayerCodeString (const e_layerCode code) {
        switch (code) {
            case LAYER_POOL_DOES_NOT_EXIST:             return "LAYER_POOL_DOES_NOT_EXIST";
            case LAYER_POOL_ALREADY_EXISTS:             return "LAYER_POOL_ALREADY_EXISTS";
            case LAYER_DOES_NOT_EXIST:                  return "LAYER_DOES_NOT_EXIST";
            case LAYER_ALREADY_EXISTS:                  return "LAYER_ALREADY_EXISTS";
            case LAYER_INSTANCE_DOES_NOT_EXIST:         return "LAYER_INSTANCE_DOES_NOT_EXIST";
            case LAYER_INSTANCE_ALREADY_EXISTS:         return "LAYER_INSTANCE_ALREADY_EXISTS";
            case LAYER_INSTANCE_RUN_DISABLED:           return "LAYER_INSTANCE_RUN_DISABLED";
            case LAYER_INSTANCE_MISSING_BINDING:        return "LAYER_INSTANCE_MISSING_BINDING";
            case VALID_REQUEST:                         return "VALID_REQUEST";
            case PENDING_REQUEST:                       return "PENDING_REQUEST";
            default:                                    return "UNDEFINED_LAYER_CODE";
        }
    }
}   // namespace Layer