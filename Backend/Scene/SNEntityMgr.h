#pragma once
#include "../Common.h"
#include "../Log/LGImpl.h"
#include "SNType.h"

namespace Scene {
    /* The entity mgr is in charge of distributing entity ids and keeping record of which ids are in use */
    class SNEntityMgr {
        private:
            struct EntityMgrInfo {
                struct Meta {
                    /* Entity ids are managed using a simple queue, where on startup the queue is initialized to contain
                     * every valid entity id up to max entities. When an entity is created it takes an id from the front
                     * of the queue, and when an entity is destroyed it puts the destroyed id at the back of the queue
                    */
                    std::queue <Entity> availableEntities;
                    /* Array of signatures where the index corresponds to the entity id */
	                std::array <Signature, g_maxEntities> signatures;
                    Entity entitiesCount;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_entityMgrInfo;

        public:
            SNEntityMgr (Log::LGImpl* logObj) {
                m_entityMgrInfo = {};

                if (logObj == nullptr) {
                    m_entityMgrInfo.resource.logObj     = new Log::LGImpl();
                    m_entityMgrInfo.state.logObjCreated = true;

                    m_entityMgrInfo.resource.logObj->initLogInfo ("Build/Log/Scene", __FILE__);
                    LOG_WARNING (m_entityMgrInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_entityMgrInfo.resource.logObj     = logObj;
                    m_entityMgrInfo.state.logObjCreated = false;
                }
            }

            void initEntityMgrInfo (void) {
                auto& meta = m_entityMgrInfo.meta;
                /* Initialize queue with available entity ids */
                for (Entity i = 0; i < g_maxEntities; i++)
                    meta.availableEntities.push (i);

                meta.signatures    = {};
                meta.entitiesCount = 0;
            }

            void updateEntitySignature (const Entity entity, const Signature entitySignature) {
                if (entity >= g_maxEntities) {
                    LOG_ERROR (m_entityMgrInfo.resource.logObj) << "Invalid entity"
                                                                << " "
                                                                << "[" << entity << "]"
                                                                << std::endl;
                    throw std::runtime_error ("Invalid entity");
                }
                m_entityMgrInfo.meta.signatures[entity] = entitySignature;
            }

            Signature getEntitySignature (const Entity entity) {
                if (entity >= g_maxEntities) {
                    LOG_ERROR (m_entityMgrInfo.resource.logObj) << "Invalid entity"
                                                                << " "
                                                                << "[" << entity << "]"
                                                                << std::endl;
                    throw std::runtime_error ("Invalid entity");
                }
                return m_entityMgrInfo.meta.signatures[entity];
            }

            Entity addEntity (void) {
                auto& meta = m_entityMgrInfo.meta;
                if (meta.entitiesCount >= g_maxEntities) {
                    LOG_ERROR (m_entityMgrInfo.resource.logObj) << "Exceeded max entities"
                                                                << " "
                                                                << "[" << meta.entitiesCount << "]"
                                                                << std::endl;
                    throw std::runtime_error ("Exceeded max entities");
                }
                /* Take an id from the front of the queue */
                auto entity = meta.availableEntities.front();
                meta.availableEntities.pop();

                ++meta.entitiesCount;
                return entity;
            }

            void removeEntity (const Entity entity) {
                auto& meta = m_entityMgrInfo.meta;
                if (entity >= g_maxEntities) {
                    LOG_ERROR (m_entityMgrInfo.resource.logObj) << "Invalid entity"
                                                                << " "
                                                                << "[" << entity << "]"
                                                                << std::endl;
                    throw std::runtime_error ("Invalid entity");
                }
                /* Invalidate the destroyed entity's signature */
                meta.signatures[entity].reset();
                /* Put the destroyed id at the back of the queue */
                meta.availableEntities.push (entity);
                --meta.entitiesCount;
            }

            void generateReport (void) {
                auto& logObj = m_entityMgrInfo.resource.logObj;

                LOG_LITE_INFO (logObj) << "\t" << "["         << std::endl;
                for (Entity i = 0; i < g_maxEntities; i++) {
                    LOG_LITE_INFO (logObj) << "\t\t";
                    LOG_LITE_INFO (logObj) << ALIGN_AND_PAD_S                    << i << ", ";
                    LOG_LITE_INFO (logObj) << m_entityMgrInfo.meta.signatures[i] << std::endl;
                }
                LOG_LITE_INFO (logObj) << "\t" << "]"         << std::endl;
            }

            ~SNEntityMgr (void) {
                if (m_entityMgrInfo.state.logObjCreated)
                    delete m_entityMgrInfo.resource.logObj;
            }
    };
}   // namespace Scene