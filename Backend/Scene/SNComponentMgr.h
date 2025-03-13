#pragma once
#include <stdexcept>
#include <unordered_map>
#include "SNComponentArrayBase.h"
#include "SNComponentArray.h"
#include "../Log/LGImpl.h"
#include "SNType.h"

namespace Scene {
    class SNComponentMgr {
        private:
            struct ComponentMgrInfo {
                struct Meta {
                    std::unordered_map <const char*, ComponentType>         types;
                    std::unordered_map <const char*, SNComponentArrayBase*> arrays;
                    ComponentType nextAvailableType;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_componentMgrInfo;

            template <typename T>
            SNComponentArray <T>* getComponentArray (void) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_componentMgrInfo.meta;
                if (meta.arrays.find (typeName) == meta.arrays.end()) {
                    LOG_ERROR (m_componentMgrInfo.resource.logObj) << "Component not registered before use"
                                                                   << " "
                                                                   << "[" << typeName << "]"
                                                                   << std::endl;
                    throw std::runtime_error ("Component not registered before use");
                }
                return static_cast <SNComponentArray <T>*> (meta.arrays[typeName]);
            }

        public:
            SNComponentMgr (Log::LGImpl* logObj) {
                m_componentMgrInfo = {};

                if (logObj == nullptr) {
                    m_componentMgrInfo.resource.logObj     = new Log::LGImpl();
                    m_componentMgrInfo.state.logObjCreated = true;

                    m_componentMgrInfo.resource.logObj->initLogInfo ("Build/Log/Scene", __FILE__);
                    LOG_WARNING (m_componentMgrInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                     << std::endl;
                }
                else {
                    m_componentMgrInfo.resource.logObj     = logObj;
                    m_componentMgrInfo.state.logObjCreated = false;
                }
            }

            void initComponentMgrInfo (void) {
                m_componentMgrInfo.meta.types             = {};
                m_componentMgrInfo.meta.arrays            = {};
                m_componentMgrInfo.meta.nextAvailableType = 0;
            }

            template <typename T>
            void registerComponent (void) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_componentMgrInfo.meta;
                if (meta.arrays.find (typeName) != meta.arrays.end()) {
                    LOG_ERROR (m_componentMgrInfo.resource.logObj) << "Component already registered"
                                                                   << " "
                                                                   << "[" << typeName << "]"
                                                                   << std::endl;
                    throw std::runtime_error ("Component already registered");
                }
                /* Create new component array */
                auto arrayObj = new SNComponentArray <T> (m_componentMgrInfo.resource.logObj);
                arrayObj->initComponentArrayInfo();

                meta.types.insert  ({typeName, meta.nextAvailableType});
                meta.arrays.insert ({typeName, arrayObj});
                ++meta.nextAvailableType;
            }

            template <typename T>
            ComponentType getComponentType (void) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_componentMgrInfo.meta;
                if (meta.arrays.find (typeName) == meta.arrays.end()) {
                    LOG_ERROR (m_componentMgrInfo.resource.logObj) << "Component not registered before use"
                                                                   << " "
                                                                   << "[" << typeName << "]"
                                                                   << std::endl;
                    throw std::runtime_error ("Component not registered before use");
                }
                return meta.types[typeName];
            }

            template <typename T>
            void addComponent (const Entity entity, const T component) {
                getComponentArray <T>()->addComponent (entity, component);
            }

            template<typename T>
            void removeComponent (const Entity entity) {
                getComponentArray <T>()->removeComponent (entity);
            }

            template<typename T>
            T* getComponent (const Entity entity) {
                return getComponentArray <T>()->getComponent (entity);
            }

            void removeEntity (const Entity entity) {
                /* Notify each component array that an entity has been destroyed, and remove it */
                for (auto const& [typeName, arrayBaseObj]: m_componentMgrInfo.meta.arrays)
                    arrayBaseObj->onRemoveEntity (entity);
            }

            void generateReport (void) {
                auto& logObj = m_componentMgrInfo.resource.logObj;

                LOG_LITE_INFO (logObj) << "\t" << "{" << std::endl;
                for (auto const& [typeName, arrayBaseObj]: m_componentMgrInfo.meta.arrays) {
                    auto type = m_componentMgrInfo.meta.types[typeName];

                    LOG_LITE_INFO (logObj) << "\t\t";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_L << typeName << ", ";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_S << type     << ", ";
                    arrayBaseObj->onGenerateReport();
                    LOG_LITE_INFO (logObj) << std::endl;
                }
                LOG_LITE_INFO (logObj) << "\t" << "}" << std::endl;
            }

            ~SNComponentMgr (void) {
                if (m_componentMgrInfo.state.logObjCreated)
                    delete m_componentMgrInfo.resource.logObj;

                for (auto const& [typeName, arrayBaseObj]: m_componentMgrInfo.meta.arrays)
                    delete arrayBaseObj;
            }
    };
}   // namespace Scene