#pragma once
#include "../Common.h"
#include "SNSystemBase.h"
#include "../Log/LGImpl.h"
#include "SNType.h"

namespace Scene {
    class SNSystemMgr {
        private:
            struct SysteMgrInfo {
                struct Meta {
                    std::unordered_map <const char*, Signature> typeNameToSignatureMap;
                    std::unordered_map <const char*, SNSystemBase*> typeNameToSystemBaseObjMap;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_systemMgrInfo;

        public:
            SNSystemMgr (Log::LGImpl* logObj) {
                m_systemMgrInfo = {};

                if (logObj == nullptr) {
                    m_systemMgrInfo.resource.logObj     = new Log::LGImpl();
                    m_systemMgrInfo.state.logObjCreated = true;

                    m_systemMgrInfo.resource.logObj->initLogInfo ("Build/Log/Scene", __FILE__);
                    LOG_WARNING (m_systemMgrInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_systemMgrInfo.resource.logObj     = logObj;
                    m_systemMgrInfo.state.logObjCreated = false;
                }
            }

            void initSystemMgrInfo (void) {
                auto& meta                      = m_systemMgrInfo.meta;
                meta.typeNameToSignatureMap     = {};
                meta.typeNameToSystemBaseObjMap = {};
            }

            template <typename T>
            T* registerSystem (void) {
                const char* typeName             = typeid (T).name();
                auto& typeNameToSystemBaseObjMap = m_systemMgrInfo.meta.typeNameToSystemBaseObjMap;

                if (typeNameToSystemBaseObjMap.find (typeName) != typeNameToSystemBaseObjMap.end()) {
                    LOG_ERROR (m_systemMgrInfo.resource.logObj) << "System already registered"
                                                                << " "
                                                                << "[" << typeName << "]"
                                                                << std::endl;
                    throw std::runtime_error ("System already registered");
                }
                auto systemObj = new T;
                typeNameToSystemBaseObjMap.insert ({typeName, systemObj});
                return systemObj;
            }

            template <typename T>
            void setSystemSignature (const Signature systemSignature) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_systemMgrInfo.meta;

                if (meta.typeNameToSystemBaseObjMap.find (typeName) == meta.typeNameToSystemBaseObjMap.end()) {
                    LOG_ERROR (m_systemMgrInfo.resource.logObj) << "System not registered before use"
                                                                << " "
                                                                << "[" << typeName << "]"
                                                                << std::endl;
                    throw std::runtime_error ("System not registered before use");
                }
                meta.typeNameToSignatureMap.insert ({typeName, systemSignature});
            }

            template <typename T>
            T* getSystem (void) {
                const char* typeName             = typeid (T).name();
                auto& typeNameToSystemBaseObjMap = m_systemMgrInfo.meta.typeNameToSystemBaseObjMap;

                if (typeNameToSystemBaseObjMap.find (typeName) == typeNameToSystemBaseObjMap.end()) {
                    LOG_ERROR (m_systemMgrInfo.resource.logObj) << "System not registered before use"
                                                                << " "
                                                                << "[" << typeName << "]"
                                                                << std::endl;
                    throw std::runtime_error ("System not registered before use");
                }
                return static_cast <T*> (typeNameToSystemBaseObjMap[typeName]);
            }

            /* When an entity’s signature has changed (due to components being added or removed), then the system’s list
             * of entities that it’s tracking needs to be updated. Similarly, if an entity that the system is tracking is
             * destroyed, then it also needs to update its list
            */
            void updateEntity (const Entity entity, const Signature entitySignature) {
                auto& meta = m_systemMgrInfo.meta;
                for (auto const& [typeName, systemBaseObj]: meta.typeNameToSystemBaseObjMap) {
                    auto systemSignature = meta.typeNameToSignatureMap[typeName];

                    /* If the updated entity's signature is of interest to the system */
                    if ((entitySignature & systemSignature) == systemSignature)
                        systemBaseObj->m_entities.insert (entity);
                    else
                        systemBaseObj->m_entities.erase  (entity);
                }
            }

            void removeEntity (const Entity entity) {
                for (auto const& [typeName, systemBaseObj]: m_systemMgrInfo.meta.typeNameToSystemBaseObjMap)
                    systemBaseObj->m_entities.erase (entity);
            }

            void generateReport (void) {
                auto& meta   = m_systemMgrInfo.meta;
                auto& logObj = m_systemMgrInfo.resource.logObj;

                LOG_LITE_INFO (logObj) << "\t" << "{" << std::endl;
                for (auto const& [typeName, systemBaseObj]: meta.typeNameToSystemBaseObjMap) {
                    auto systemSignature = meta.typeNameToSignatureMap[typeName];
                    std::string spacer   = "";

                    LOG_LITE_INFO (logObj) << "\t\t";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_L << typeName        << ", ";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_M << systemSignature << ", ";

                    LOG_LITE_INFO (logObj) << "[";
                    for (auto const& entity: systemBaseObj->m_entities) {
                        LOG_LITE_INFO (logObj) << spacer << entity;
                        spacer = ", ";
                    }
                    LOG_LITE_INFO (logObj) << "]";
                    LOG_LITE_INFO (logObj) << std::endl;
                }
                LOG_LITE_INFO (logObj) << "\t" << "}" << std::endl;
            }

            ~SNSystemMgr (void) {
                for (auto const& [typeName, systemBaseObj]: m_systemMgrInfo.meta.typeNameToSystemBaseObjMap)
                    delete systemBaseObj;

                if (m_systemMgrInfo.state.logObjCreated)
                    delete m_systemMgrInfo.resource.logObj;
            }
    };
}   // namespace Scene