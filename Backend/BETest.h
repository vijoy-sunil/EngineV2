#pragma once
#include "Layer/LYPoolMgr.h"
#include "Log/LGImpl.h"
#include "Log/LGMacro.h"

namespace Test {
    void readyLogLayerTest (void) {
        /* Create pool, layer and instance */
        auto layerCode = Layer::PENDING_REQUEST;
        auto& poolMgr  = Layer::LYPoolMgr::getLayerPoolMgr();
        auto pool      = poolMgr.createLayerPool  ("LOG_POOL",          layerCode);
        pool->createLayer                         (0,                   layerCode);
        pool->createLayerInstance                 (0, "LOG_INSTANCE_0", layerCode);
        auto logInstance = pool->getLayerInstance (0, "LOG_INSTANCE_0", layerCode);

        /* Create log object */
        auto logObj    = new Log::LGImpl          (Log::LOG_LEVEL_INFO,
                                                   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE,
                                                   "Build/Log/Test",
                                                   "logLayer.txt");
        /* Populate instance */
        logInstance->resource.instance = logObj;
        logInstance->state.runDisabled = true;

        /* Generate report */
        poolMgr.generateReport();
        pool->generateReport();
    }

    void runLogLayerTest (void) {
        /* Get pool and instance */
        auto layerCode   = Layer::PENDING_REQUEST;
        auto& poolMgr    = Layer::LYPoolMgr::getLayerPoolMgr();
        auto pool        = poolMgr.getLayerPool       ("LOG_POOL",          layerCode);
        auto logInstance = pool->getLayerInstance     (0, "LOG_INSTANCE_0", layerCode);
        auto logObj      = static_cast <Log::LGImpl*> (logInstance->resource.instance);

        /* Write log messages */
        logObj->LOG_INFO ("This is an info message");
        logObj->LOG_WARN ("This is a warning message");
        logObj->LOG_ERRO ("This is an error message");

        /* Destroy layer and pool */
        pool->destroyLayer       (0, layerCode);
        pool->generateReport();
        poolMgr.destroyLayerPool ("LOG_POOL", layerCode);
        poolMgr.generateReport();
    }
}   // namespace Test