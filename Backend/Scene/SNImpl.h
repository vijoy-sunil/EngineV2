#pragma once
#include "../Common.h"
#include "SNEntityMgr.h"
#include "SNComponentMgr.h"
#include "SNSystemMgr.h"
#include "../Log/LGImpl.h"
#include "SNType.h"

namespace Scene {
    /* https://austinmorlan.com/posts/entity_component_system/ */
    class SNImpl {
        private:
            struct SceneInfo {
                struct Resource {
                    SNEntityMgr* entityMgrObj;
                    SNComponentMgr* componentMgrObj;
                    SNSystemMgr* systemMgrObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_sceneInfo;

        public:
            SNImpl (void) {
                m_sceneInfo = {};

                auto& logObj = m_sceneInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/Scene",       __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);

                m_sceneInfo.resource.entityMgrObj    = new SNEntityMgr    (logObj);
                m_sceneInfo.resource.entityMgrObj->initEntityMgrInfo();
                m_sceneInfo.resource.componentMgrObj = new SNComponentMgr (logObj);
                m_sceneInfo.resource.componentMgrObj->initComponentMgrInfo();
                m_sceneInfo.resource.systemMgrObj    = new SNSystemMgr    (logObj);
                m_sceneInfo.resource.systemMgrObj->initSystemMgrInfo();
            }

            void initSceneInfo (void) {
            }

            /* Entity methods */
            Signature getEntitySignature (const Entity entity) {
                return m_sceneInfo.resource.entityMgrObj->getEntitySignature (entity);
            }

            Entity addEntity (void) {
                return m_sceneInfo.resource.entityMgrObj->addEntity();
            }

            void removeEntity (const Entity entity) {
                auto& resource = m_sceneInfo.resource;
                resource.entityMgrObj->removeEntity    (entity);
                resource.componentMgrObj->removeEntity (entity);
                resource.systemMgrObj->removeEntity    (entity);
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
                auto& resource = m_sceneInfo.resource;
                resource.componentMgrObj->addComponent <T> (entity, component);

                auto entitySignature = resource.entityMgrObj->getEntitySignature (entity);
                auto componentType   = resource.componentMgrObj->getComponentType <T>();
                entitySignature.set (componentType, true);

                resource.entityMgrObj->updateEntitySignature (entity, entitySignature);
                resource.systemMgrObj->updateEntity          (entity, entitySignature);
            }

            template <typename T>
            void removeComponent (const Entity entity) {
                auto& resource = m_sceneInfo.resource;
                resource.componentMgrObj->removeComponent <T> (entity);

                auto entitySignature = resource.entityMgrObj->getEntitySignature (entity);
                auto componentType   = resource.componentMgrObj->getComponentType <T>();
                entitySignature.set (componentType, false);

                resource.entityMgrObj->updateEntitySignature (entity, entitySignature);
                resource.systemMgrObj->updateEntity          (entity, entitySignature);
            }

            template <typename T>
            T* getComponent (const Entity entity) {
                return m_sceneInfo.resource.componentMgrObj->getComponent <T> (entity);
            }

            /* System methods */
            template <typename T>
            T* registerSystem (void) {
                return m_sceneInfo.resource.systemMgrObj->registerSystem <T>();
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
                auto& resource = m_sceneInfo.resource;

                LOG_LITE_INFO (resource.logObj) << "{"                     << std::endl;
                LOG_LITE_INFO (resource.logObj) << "\t" << "Entity mgr"    << std::endl;
                resource.entityMgrObj->generateReport();
                LOG_LITE_INFO (resource.logObj) << "\t" << "Component mgr" << std::endl;
                resource.componentMgrObj->generateReport();
                LOG_LITE_INFO (resource.logObj) << "\t" << "System mgr"    << std::endl;
                resource.systemMgrObj->generateReport();
                LOG_LITE_INFO (resource.logObj) << "}"                     << std::endl;
            }

            ~SNImpl (void) {
                auto& resource = m_sceneInfo.resource;
                delete resource.systemMgrObj;
                delete resource.componentMgrObj;
                delete resource.entityMgrObj;
                delete resource.logObj;
            }
    };
}   // namespace Scene