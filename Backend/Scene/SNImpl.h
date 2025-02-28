#pragma once
#include "SNEntityMgr.h"
#include "SNComponentMgr.h"
#include "SNSystemMgr.h"
#include "../Log/LGImpl.h"
#include "../Log/LGEnum.h"
#include "SNType.h"

namespace Scene {
    /* https://austinmorlan.com/posts/entity_component_system/ */
    class SNImpl {
        private:
            struct SceneInfo {
                struct Resource {
                    Log::LGImpl*    logObj;
                    SNEntityMgr*    entityMgrObj;
                    SNComponentMgr* componentMgrObj;
                    SNSystemMgr*    systemMgrObj;
                } resource;
            } m_sceneInfo;

        public:
            SNImpl (void) {
                m_sceneInfo = {};

                m_sceneInfo.resource.logObj          = new Log::LGImpl();
                m_sceneInfo.resource.logObj->initLogInfo     ("Build/Log/Scene",      __FILE__);
                m_sceneInfo.resource.logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                m_sceneInfo.resource.logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE |
                                                                                      Log::LOG_SINK_FILE);
                m_sceneInfo.resource.logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE |
                                                                                      Log::LOG_SINK_FILE);

                m_sceneInfo.resource.entityMgrObj    = new SNEntityMgr    (m_sceneInfo.resource.logObj);
                m_sceneInfo.resource.componentMgrObj = new SNComponentMgr (m_sceneInfo.resource.logObj);
                m_sceneInfo.resource.systemMgrObj    = new SNSystemMgr    (m_sceneInfo.resource.logObj);
            }

            void initSceneInfo (void) {
                m_sceneInfo.resource.entityMgrObj->initEntityMgrInfo();
                m_sceneInfo.resource.componentMgrObj->initComponentMgrInfo();
                m_sceneInfo.resource.systemMgrObj->initSystemMgrInfo();
            }

            /* Entity methods */
            Entity addEntity (void) {
                return m_sceneInfo.resource.entityMgrObj->addEntity();
            }

            void removeEntity (const Entity entity) {
                m_sceneInfo.resource.entityMgrObj->removeEntity    (entity);
                m_sceneInfo.resource.componentMgrObj->removeEntity (entity);
                m_sceneInfo.resource.systemMgrObj->removeEntity    (entity);
            }

            /* Component methods */
            template <typename T>
            void registerComponent (void) {
                m_sceneInfo.resource.componentMgrObj->registerComponent <T>();
            }

            template <typename T>
            ComponentType getComponentType (void) {
                return m_sceneInfo.resource.componentMgrObj->getComponentType <T>();
            }

            template <typename T>
            void addComponent (const Entity entity, const T component) {
                m_sceneInfo.resource.componentMgrObj->addComponent <T> (entity, component);

                auto entitySignature = m_sceneInfo.resource.entityMgrObj->getEntitySignature (entity);
                auto componentType   = m_sceneInfo.resource.componentMgrObj->getComponentType <T>();
                entitySignature.set (componentType, true);

                m_sceneInfo.resource.entityMgrObj->setEntitySignature (entity, entitySignature);
                m_sceneInfo.resource.systemMgrObj->updateEntity       (entity, entitySignature);
            }

            template <typename T>
            void removeComponent (const Entity entity) {
                m_sceneInfo.resource.componentMgrObj->removeComponent <T> (entity);

                auto entitySignature = m_sceneInfo.resource.entityMgrObj->getEntitySignature (entity);
                auto componentType   = m_sceneInfo.resource.componentMgrObj->getComponentType <T>();
                entitySignature.set (componentType, false);

                m_sceneInfo.resource.entityMgrObj->setEntitySignature (entity, entitySignature);
                m_sceneInfo.resource.systemMgrObj->updateEntity       (entity, entitySignature);
            }

            template <typename T>
            T* getComponent (const Entity entity) {
                return m_sceneInfo.resource.componentMgrObj->getComponent <T> (entity);
            }

            /* System methods */
            template <typename T>
            void registerSystem (void) {
                m_sceneInfo.resource.systemMgrObj->registerSystem <T>();
            }

            template <typename T>
            void setSystemSignature (const Signature systemSignature) {
                m_sceneInfo.resource.systemMgrObj->setSystemSignature <T> (systemSignature);
            }

            template <typename T>
            T* getSystem (void) {
                return m_sceneInfo.resource.systemMgrObj->getSystem <T>();
            }

            void generateReport (void) {
                auto& logObj = m_sceneInfo.resource.logObj;

                LOG_LITE_INFO (logObj) << "{"                               << std::endl;
                LOG_LITE_INFO (logObj) << "\t"         << "Entity mgr"      << std::endl;
                m_sceneInfo.resource.entityMgrObj->generateReport();
                LOG_LITE_INFO (logObj) << "\t"         << "Component mgr"   << std::endl;
                m_sceneInfo.resource.componentMgrObj->generateReport();
                LOG_LITE_INFO (logObj) << "\t"         << "System mgr"      << std::endl;
                m_sceneInfo.resource.systemMgrObj->generateReport();
                LOG_LITE_INFO (logObj) << "}"                               << std::endl;
            }

            ~SNImpl (void) {
                delete m_sceneInfo.resource.logObj;
                delete m_sceneInfo.resource.entityMgrObj;
                delete m_sceneInfo.resource.componentMgrObj;
                delete m_sceneInfo.resource.systemMgrObj;
            }
    };
}   // namespace Scene