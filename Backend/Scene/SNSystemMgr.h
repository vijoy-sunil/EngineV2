#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <iomanip>
#include "SNSystemBase.h"
#include "../Log/LGImpl.h"
#include "SNType.h"

namespace Scene {
    class SNSystemMgr {
        private:
            struct SysteMgrInfo {
                struct Meta {
                    std::unordered_map <const char*, Signature>     signatures;
                    std::unordered_map <const char*, SNSystemBase*> systems;
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
                m_systemMgrInfo.meta.signatures = {};
                m_systemMgrInfo.meta.systems    = {};
            }

            template <typename T>
            void registerSystem (void) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_systemMgrInfo.meta;
                if (meta.systems.find (typeName) != meta.systems.end()) {
                    LOG_ERROR (m_systemMgrInfo.resource.logObj) << "System already registered"
                                                                << " "
                                                                << "[" << typeName << "]"
                                                                << std::endl;
                    throw std::runtime_error ("System already registered");
                }
                auto systemObj = new T;
                meta.systems.insert ({typeName, systemObj});
            }

            template <typename T>
            void setSystemSignature (const Signature systemSignature) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_systemMgrInfo.meta;
                if (meta.systems.find (typeName) == meta.systems.end()) {
                    LOG_ERROR (m_systemMgrInfo.resource.logObj) << "System not registered before use"
                                                                << " "
                                                                << "[" << typeName << "]"
                                                                << std::endl;
                    throw std::runtime_error ("System not registered before use");
                }
                meta.signatures.insert ({typeName, systemSignature});
            }

            template <typename T>
            T* getSystem (void) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_systemMgrInfo.meta;
                if (meta.systems.find (typeName) == meta.systems.end()) {
                    LOG_ERROR (m_systemMgrInfo.resource.logObj) << "System not registered before use"
                                                                << " "
                                                                << "[" << typeName << "]"
                                                                << std::endl;
                    throw std::runtime_error ("System not registered before use");
                }
                return static_cast <T*> (meta.systems[typeName]);
            }

            /* When an entity’s signature has changed (due to components being added or removed), then the system’s list
             * of entities that it’s tracking needs to be updated. Similarly, if an entity that the system is tracking is
             * destroyed, then it also needs to update its list
            */
            void updateEntity (const Entity entity, const Signature entitySignature) {
                for (auto const& [typeName, systemBaseObj]: m_systemMgrInfo.meta.systems) {
                    auto systemSignature = m_systemMgrInfo.meta.signatures[typeName];

                    /* If the updated entity's signature is of interest to the system */
                    if ((entitySignature & systemSignature) == systemSignature)
                        systemBaseObj->m_entities.insert (entity);
                    else
                        systemBaseObj->m_entities.erase  (entity);
                }
            }

            void removeEntity (const Entity entity) {
                for (auto const& [typeName, systemBaseObj]: m_systemMgrInfo.meta.systems)
                    systemBaseObj->m_entities.erase (entity);
            }

            void generateReport (void) {
                auto& logObj       = m_systemMgrInfo.resource.logObj;
                std::string spacer = "";

                LOG_LITE_INFO (logObj) << "\t" << "{" << std::endl;
                for (auto const& [typeName, systemBaseObj]: m_systemMgrInfo.meta.systems) {
                    auto systemSignature = m_systemMgrInfo.meta.signatures[typeName];

                    LOG_LITE_INFO (logObj) << "\t\t";
                    LOG_LITE_INFO (logObj) << std::left << std::setw (30) << typeName        << ", ";
                    LOG_LITE_INFO (logObj) << std::left << std::setw (15) << systemSignature << ", ";

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
                if (m_systemMgrInfo.state.logObjCreated)
                    delete m_systemMgrInfo.resource.logObj;

                for (auto const& [typeName, systemBaseObj]: m_systemMgrInfo.meta.systems)
                    delete systemBaseObj;
            }
    };
}   // namespace Scene