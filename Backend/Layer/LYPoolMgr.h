#pragma once
#include <unordered_map>
#include "LYPool.h"
#include "../Log/LGImpl.h"
#include "LYEnum.h"

namespace Layer {
    class LYPoolMgr: public LYPool {
        private:
            std::unordered_map <const char*, LYPool*> m_layerPools;
            Log::LGImpl* m_logObj;

            /* Private default constructor */
            LYPoolMgr (void) {
                m_logObj = new Log::LGImpl();
                m_logObj->initLogInfo     ("Build/Log/Layer",      __FILE__);
                m_logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                m_logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_NONE);
                m_logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_NONE);
            }

        public:
            /* Delete copy constructor and assignment operator */
            LYPoolMgr       (LYPoolMgr const&) = delete;
            void operator = (LYPoolMgr const&) = delete;

            /* Static method to access the singleton instance */
            static LYPoolMgr& getLayerPoolMgr (void) {
                static LYPoolMgr mgrObj;
                return mgrObj;
            }

            LYPool* addLayerPool (const char* poolId, e_layerCode& code) {
                if (m_layerPools.find (poolId) != m_layerPools.end())
                    code = LAYER_POOL_ALREADY_EXISTS;
                else {
                    code = VALID_REQUEST;
                    m_layerPools.insert ({poolId, new LYPool()});
                }
                return m_layerPools[poolId];
            }

            LYPool* getLayerPool (const char* poolId, e_layerCode& code) {
                if (m_layerPools.find (poolId) != m_layerPools.end()) {
                    code = VALID_REQUEST;
                    return m_layerPools[poolId];
                }
                else {
                    code = LAYER_POOL_DOES_NOT_EXIST;
                    return nullptr;
                }
            }

            /* 3-step destroy */
            void destroyLayerPool (const char* poolId, e_layerCode& code) {
                if (m_layerPools.find (poolId) != m_layerPools.end()) {
                    code = VALID_REQUEST;
                    m_layerPools[poolId]->destroyAllLayers();   /* (1) */
                    delete m_layerPools[poolId];                /* (2) */
                    m_layerPools.erase (poolId);                /* (3) */
                }
                else
                    code = LAYER_POOL_DOES_NOT_EXIST;
            }

            void generateReport (void) {
                LOG_LITE_INFO (m_logObj) << "{"        << std::endl;

                for (auto const& [poolId, pool]: m_layerPools) {
                    LOG_LITE_INFO (m_logObj) << "\t";
                    LOG_LITE_INFO (m_logObj) << poolId << std::endl;
                    pool->generateReportEXT (m_logObj);
                }
                LOG_LITE_INFO (m_logObj) << "}"        << std::endl;
            }

            ~LYPoolMgr (void) {
                delete m_logObj;
            }
    };
}   // namespace Layer