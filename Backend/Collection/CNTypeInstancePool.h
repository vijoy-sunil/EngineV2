#pragma once
#include <string>
#include <unordered_map>
#include <iomanip>
#include <vector>
#include "CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"

namespace Collection {
    class CNTypeInstancePool {
        private:
            struct TypeInstancePoolInfo {
                struct Meta {
                    std::unordered_map <const char*, size_t> instanceIdToIdxMap;
                    std::vector <CNTypeInstanceBase*> pool;
                    size_t nextAvailableIdx;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_typeInstancePoolInfo;

        public:
            CNTypeInstancePool (Log::LGImpl* logObj) {
                m_typeInstancePoolInfo = {};

                if (logObj == nullptr) {
                    m_typeInstancePoolInfo.resource.logObj     = new Log::LGImpl();
                    m_typeInstancePoolInfo.state.logObjCreated = true;

                    m_typeInstancePoolInfo.resource.logObj->initLogInfo ("Build/Log/Collection", __FILE__);
                    LOG_WARNING (m_typeInstancePoolInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                         << std::endl;
                }
                else {
                    m_typeInstancePoolInfo.resource.logObj     = logObj;
                    m_typeInstancePoolInfo.state.logObjCreated = false;
                }
            }

            void initTypeInstancePoolInfo (void) {
                m_typeInstancePoolInfo.meta.instanceIdToIdxMap = {};
                m_typeInstancePoolInfo.meta.pool               = {};
                m_typeInstancePoolInfo.meta.nextAvailableIdx   = 0;
            }

            std::vector <CNTypeInstanceBase*>& getCollectionTypeInstancePool (void) {
                return m_typeInstancePoolInfo.meta.pool;
            }

            void addCollectionTypeInstance (const char* instanceId, CNTypeInstanceBase* instanceBaseObj) {
                auto& meta = m_typeInstancePoolInfo.meta;
                if (meta.instanceIdToIdxMap.find (instanceId) != meta.instanceIdToIdxMap.end()) {
                    LOG_ERROR (m_typeInstancePoolInfo.resource.logObj) << "Collection type instance already exists"
                                                                       << " "
                                                                       << "[" << instanceId << "]"
                                                                       << std::endl;
                    throw std::runtime_error ("Collection type instance already exists");
                }
                size_t idx                          = meta.nextAvailableIdx;
                meta.instanceIdToIdxMap[instanceId] = idx;
                meta.pool.push_back (instanceBaseObj);

                ++meta.nextAvailableIdx;
            }

            CNTypeInstanceBase* removeCollectionTypeInstance (const char* instanceId) {
                auto& meta = m_typeInstancePoolInfo.meta;
                if (meta.instanceIdToIdxMap.find (instanceId) == meta.instanceIdToIdxMap.end()) {
                    LOG_ERROR (m_typeInstancePoolInfo.resource.logObj) << "Collection type instance does not exist"
                                                                       << " "
                                                                       << "[" << instanceId << "]"
                                                                       << std::endl;
                    throw std::runtime_error ("Collection type instance does not exist");
                }
                size_t idx           = meta.instanceIdToIdxMap[instanceId];
                auto instanceBaseObj = meta.pool[idx];
                auto it              = meta.pool.begin();

                meta.instanceIdToIdxMap.erase (instanceId);
                meta.pool.erase               (it + idx);
                return instanceBaseObj;
            }

            CNTypeInstanceBase* getCollectionTypeInstance (const char* instanceId) {
                auto& meta = m_typeInstancePoolInfo.meta;
                if (meta.instanceIdToIdxMap.find (instanceId) == meta.instanceIdToIdxMap.end()) {
                    LOG_ERROR (m_typeInstancePoolInfo.resource.logObj) << "Collection type instance does not exist"
                                                                       << " "
                                                                       << "[" << instanceId << "]"
                                                                       << std::endl;
                    throw std::runtime_error ("Collection type instance does not exist");
                }
                size_t idx           = meta.instanceIdToIdxMap[instanceId];
                auto instanceBaseObj = meta.pool[idx];

                return instanceBaseObj;
            }

            void generateReport (void) {
                auto& logObj       = m_typeInstancePoolInfo.resource.logObj;
                auto& meta         = m_typeInstancePoolInfo.meta;
                std::string spacer = "";

                LOG_LITE_INFO (logObj) << "[";
                for (auto const& [instanceId, idx]: meta.instanceIdToIdxMap) {
                    LOG_LITE_INFO (logObj) << spacer    << idx            << "/";
                    LOG_LITE_INFO (logObj) << std::left << std::setw (15) << instanceId;
                    spacer = ", ";
                }
                LOG_LITE_INFO (logObj) << "]";
            }

            ~CNTypeInstancePool (void) {
                if (m_typeInstancePoolInfo.state.logObjCreated)
                    delete m_typeInstancePoolInfo.resource.logObj;
            }
    };
}   // namespace Collection