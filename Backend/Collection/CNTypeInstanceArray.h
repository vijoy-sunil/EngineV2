#pragma once
#include <string>
#include <unordered_map>
#include <iomanip>
#include <array>
#include "CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"

namespace Collection {
    const uint32_t g_maxTypeInstances = 32;

    class CNTypeInstanceArray {
        private:
            struct TypeInstanceArrayInfo {
                struct Meta {
                    std::array <CNTypeInstanceBase*, g_maxTypeInstances> array;
                    std::unordered_map <const char*, size_t> instanceIdToIdxMap;
                    std::unordered_map <size_t, const char*> idxToInstanceIdMap;
                    size_t nextAvailableIdx;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_typeInstanceArrayInfo;

        public:
            CNTypeInstanceArray (Log::LGImpl* logObj) {
                m_typeInstanceArrayInfo = {};

                if (logObj == nullptr) {
                    m_typeInstanceArrayInfo.resource.logObj     = new Log::LGImpl();
                    m_typeInstanceArrayInfo.state.logObjCreated = true;

                    m_typeInstanceArrayInfo.resource.logObj->initLogInfo ("Build/Log/Collection", __FILE__);
                    LOG_WARNING (m_typeInstanceArrayInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                          << std::endl;
                }
                else {
                    m_typeInstanceArrayInfo.resource.logObj     = logObj;
                    m_typeInstanceArrayInfo.state.logObjCreated = false;
                }
            }

            void initTypeInstanceArrayInfo (void) {
                m_typeInstanceArrayInfo.meta.array              = {};
                m_typeInstanceArrayInfo.meta.instanceIdToIdxMap = {};
                m_typeInstanceArrayInfo.meta.idxToInstanceIdMap = {};
                m_typeInstanceArrayInfo.meta.nextAvailableIdx   = 0;
            }

            std::array <CNTypeInstanceBase*, g_maxTypeInstances>& getCollectionTypeInstanceArray (void) {
                return m_typeInstanceArrayInfo.meta.array;
            }

            void addCollectionTypeInstance (const char* instanceId, CNTypeInstanceBase* instanceBaseObj) {
                auto& meta = m_typeInstanceArrayInfo.meta;
                if (meta.instanceIdToIdxMap.find (instanceId) != meta.instanceIdToIdxMap.end()) {
                    LOG_ERROR (m_typeInstanceArrayInfo.resource.logObj) << "Collection type instance already exists"
                                                                        << " "
                                                                        << "[" << instanceId << "]"
                                                                        << std::endl;
                    throw std::runtime_error ("Collection type instance already exists");
                }
                size_t idx                          = meta.nextAvailableIdx;
                meta.array[idx]                     = instanceBaseObj;
                meta.instanceIdToIdxMap[instanceId] = idx;
                meta.idxToInstanceIdMap[idx]        = instanceId;

                ++meta.nextAvailableIdx;
            }

            CNTypeInstanceBase* removeCollectionTypeInstance (const char* instanceId) {
                auto& meta = m_typeInstanceArrayInfo.meta;
                if (meta.instanceIdToIdxMap.find (instanceId) == meta.instanceIdToIdxMap.end()) {
                    LOG_ERROR (m_typeInstanceArrayInfo.resource.logObj) << "Collection type instance does not exist"
                                                                        << " "
                                                                        << "[" << instanceId << "]"
                                                                        << std::endl;
                    throw std::runtime_error ("Collection type instance does not exist");
                }
                size_t removeIdx                        = meta.instanceIdToIdxMap[instanceId];
                size_t lastIdx                          = meta.nextAvailableIdx - 1;
                /* Save to return before overwrite */
                auto instanceBaseObj                    = meta.array[removeIdx];
                meta.array[removeIdx]                   = meta.array[lastIdx];

                auto lastInstanceId                     = meta.idxToInstanceIdMap[lastIdx];
                meta.instanceIdToIdxMap[lastInstanceId] = removeIdx;
                meta.idxToInstanceIdMap[removeIdx]      = lastInstanceId;

                meta.instanceIdToIdxMap.erase (instanceId);
                meta.idxToInstanceIdMap.erase (lastIdx);

                --meta.nextAvailableIdx;
                return instanceBaseObj;
            }

            CNTypeInstanceBase* getCollectionTypeInstance (const char* instanceId) {
                auto& meta = m_typeInstanceArrayInfo.meta;
                if (meta.instanceIdToIdxMap.find (instanceId) == meta.instanceIdToIdxMap.end()) {
                    LOG_ERROR (m_typeInstanceArrayInfo.resource.logObj) << "Collection type instance does not exist"
                                                                        << " "
                                                                        << "[" << instanceId << "]"
                                                                        << std::endl;
                    throw std::runtime_error ("Collection type instance does not exist");
                }
                return meta.array[meta.instanceIdToIdxMap[instanceId]];
            }

            void generateReport (void) {
                auto& logObj       = m_typeInstanceArrayInfo.resource.logObj;
                auto& meta         = m_typeInstanceArrayInfo.meta;
                std::string spacer = "";

                LOG_LITE_INFO (logObj) << "[";
                for (uint32_t i = 0; i < g_maxTypeInstances; i++) {
                    if (meta.idxToInstanceIdMap.find (i) != meta.idxToInstanceIdMap.end()) {
                        LOG_LITE_INFO (logObj) << spacer;
                        LOG_LITE_INFO (logObj) << std::left << std::setw (30) << meta.idxToInstanceIdMap[i];
                        spacer = ", ";
                    }
                }
                LOG_LITE_INFO (logObj) << "]";
            }

            ~CNTypeInstanceArray (void) {
                if (m_typeInstanceArrayInfo.state.logObjCreated)
                    delete m_typeInstanceArrayInfo.resource.logObj;
            }
    };
}   // namespace Collection