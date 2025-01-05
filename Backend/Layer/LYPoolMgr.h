#pragma once
#include <unordered_map>
#include <iostream>
#include "LYPool.h"

namespace Layer {
    class LYPoolMgr: public LYPool {
        private:
            std::unordered_map <const char*, LYPool*> m_layerPools;
            /* Private default constructor */
            LYPoolMgr (void) {}

        public:
            /* Delete copy constructor and assignment operator */
            LYPoolMgr       (LYPoolMgr const&) = delete;
            void operator = (LYPoolMgr const&) = delete;

            /* Static method to access the singleton instance */
            static LYPoolMgr& getLayerPoolMgr (void) {
                static LYPoolMgr mgrObj;
                return mgrObj;
            }

            LYPool* createLayerPool (const char* poolId, e_layerCode& code) {
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
                std::cout << "Layer pool mgr report" << std::endl;
                std::cout << "{"                     << std::endl;
                for (auto const& [poolId, pool]: m_layerPools) {
                    std::cout << "\t";
                    std::cout << poolId              << std::endl;
                }
                std::cout << "}"                     << std::endl;
            }
    };
}   // namespace Layer