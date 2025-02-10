#pragma once
#include <iomanip>
#include <array>
#include <queue>
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
                /* Initialize queue with available entity ids */
                for (Entity i = 0; i < g_maxEntities; i++)
                    m_entityMgrInfo.meta.availableEntities.push (i);

                m_entityMgrInfo.meta.signatures    = {};
                m_entityMgrInfo.meta.entitiesCount = 0;
            }

            void setEntitySignature (const Entity entity, const Signature entitySignature) {
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
                if (m_entityMgrInfo.meta.entitiesCount >= g_maxEntities) {
                    LOG_ERROR (m_entityMgrInfo.resource.logObj) << "Exceeded max entities count"
                                                                << " "
                                                                << "[" << m_entityMgrInfo.meta.entitiesCount << "]"
                                                                << std::endl;
                    throw std::runtime_error ("Exceeded max entities count");
                }
                /* Take an id from the front of the queue */
                auto entity = m_entityMgrInfo.meta.availableEntities.front();
                m_entityMgrInfo.meta.availableEntities.pop();

                ++m_entityMgrInfo.meta.entitiesCount;
                return entity;
            }

            void removeEntity (const Entity entity) {
                if (entity >= g_maxEntities) {
                    LOG_ERROR (m_entityMgrInfo.resource.logObj) << "Invalid entity"
                                                                << " "
                                                                << "[" << entity << "]"
                                                                << std::endl;
                    throw std::runtime_error ("Invalid entity");
                }
                /* Invalidate the destroyed entity's signature */
		        m_entityMgrInfo.meta.signatures[entity].reset();
                /* Put the destroyed id at the back of the queue */
		        m_entityMgrInfo.meta.availableEntities.push (entity);
		        --m_entityMgrInfo.meta.entitiesCount;
            }

            void generateReport (void) {
                auto& logObj     = m_entityMgrInfo.resource.logObj;
                auto& signatures = m_entityMgrInfo.meta.signatures;

                LOG_LITE_INFO (logObj) << "\t" << "["       << std::endl;
                for (size_t i = 0; i < signatures.size(); i++) {
                    LOG_LITE_INFO (logObj) << "\t\t";
                    LOG_LITE_INFO (logObj) << std::left     << std::setw (3) << i << ", ";
                    LOG_LITE_INFO (logObj) << signatures[i] << std::endl;
                }
                LOG_LITE_INFO (logObj) << "\t" << "]"       << std::endl;
            }

            ~SNEntityMgr (void) {
                if (m_entityMgrInfo.state.logObjCreated)
                    delete m_entityMgrInfo.resource.logObj;
            }
    };
}   // namespace Scene