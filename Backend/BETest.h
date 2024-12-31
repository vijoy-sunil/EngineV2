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
        auto logObj = new Log::LGImpl();
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

        /* Update log configs */
        logObj->updateLogConfig    (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_CONSOLE);
        logObj->updateLogConfig    (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_FILE);
        logObj->updateLogConfig    (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_NONE);
        logObj->updateSaveLocation ("Build/Log/Test", "logLayer.txt");

        /* Write log messages */
        logObj->LOG_INFO ("This is an info message for console");
        logObj->LOG_WARN ("This is a warning message for file");
        logObj->LOG_ERRO ("This shouldn't be logged!");

        /* Destroy layer and pool */
        pool->destroyLayer       (0, layerCode);
        pool->generateReport();
        poolMgr.destroyLayerPool ("LOG_POOL", layerCode);
        poolMgr.generateReport();
    }
}   // namespace Test