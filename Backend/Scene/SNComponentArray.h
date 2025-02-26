#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <array>
#include "SNComponentArrayBase.h"
#include "../Log/LGImpl.h"
#include "SNType.h"

namespace Scene {
    template <typename T>
    class SNComponentArray: public SNComponentArrayBase {
        private:
            struct ComponentArrayInfo {
                struct Meta {
                    /* The packed array of components (of generic type T) matches the maximum number of entities allowed
                     * to exist simultaneously, so that each entity has a unique spot
                    */
                    std::array <T, g_maxEntities> array;
                    /* When accessing the array, you use the entity id to look up the actual array index. Then, when an
                     * entity is destroyed, you take the last valid element in the array and move it into the deleted
                     * entity’s spot and update the map so that the entity id now points to the correct position. There
                     * is also a map from the array index to an entity id so that, when moving the last array element,
                     * you know which entity was using that index and can update its map
                    */
                    std::unordered_map <Entity, size_t> entityToIdxMap;
                    std::unordered_map <size_t, Entity> idxToEntityMap;
                    size_t nextAvailableIdx;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_componentArrayInfo;

        public:
            SNComponentArray (Log::LGImpl* logObj) {
                m_componentArrayInfo = {};

                if (logObj == nullptr) {
                    m_componentArrayInfo.resource.logObj     = new Log::LGImpl();
                    m_componentArrayInfo.state.logObjCreated = true;

                    m_componentArrayInfo.resource.logObj->initLogInfo ("Build/Log/Scene", __FILE__);
                    LOG_WARNING (m_componentArrayInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                       << std::endl;
                }
                else {
                    m_componentArrayInfo.resource.logObj     = logObj;
                    m_componentArrayInfo.state.logObjCreated = false;
                }
            }

            void initComponentArrayInfo (void) {
                m_componentArrayInfo.meta.array            = {};
                m_componentArrayInfo.meta.entityToIdxMap   = {};
                m_componentArrayInfo.meta.idxToEntityMap   = {};
                m_componentArrayInfo.meta.nextAvailableIdx = 0;
            }

            void addComponent (const Entity entity, const T component) {
                auto& meta = m_componentArrayInfo.meta;
                if (meta.entityToIdxMap.find (entity) != meta.entityToIdxMap.end()) {
                    LOG_ERROR (m_componentArrayInfo.resource.logObj) << "Component already exists"
                                                                     << " "
                                                                     << "[" << entity << "]"
                                                                     << std::endl;
                    throw std::runtime_error ("Component already exists");
                }
                size_t idx                  = meta.nextAvailableIdx;
                meta.array[idx]             = component;
                meta.entityToIdxMap[entity] = idx;
                meta.idxToEntityMap[idx]    = entity;

                ++meta.nextAvailableIdx;
            }

            void removeComponent (const Entity entity) {
                auto& meta = m_componentArrayInfo.meta;
                if (meta.entityToIdxMap.find (entity) == meta.entityToIdxMap.end()) {
                    LOG_ERROR (m_componentArrayInfo.resource.logObj) << "Component does not exist"
                                                                     << " "
                                                                     << "[" << entity << "]"
                                                                     << std::endl;
                    throw std::runtime_error ("Component does not exist");
                }
                size_t removeIdx                = meta.entityToIdxMap[entity];
                size_t lastIdx                  = meta.nextAvailableIdx - 1;
                meta.array[removeIdx]           = meta.array[lastIdx];

                auto lastEntity                 = meta.idxToEntityMap[lastIdx];
                meta.entityToIdxMap[lastEntity] = removeIdx;
                meta.idxToEntityMap[removeIdx]  = lastEntity;

                meta.entityToIdxMap.erase (entity);
                meta.idxToEntityMap.erase (lastIdx);

		        --meta.nextAvailableIdx;
            }

            T* getComponent (const Entity entity) {
                /* The unordered_map does have a performance penalty because when you want to get the index of a component
                 * to grab it from the contiguous array, you have to request it from the unordered_map which is not
                 * contiguous. However, they have the nice property of supporting find(), insert(), and delete(), which
                 * allow for asserting validity without "if (valid)" checks and it's a bit cleaner
                */
                auto& meta = m_componentArrayInfo.meta;
                if (meta.entityToIdxMap.find (entity) == meta.entityToIdxMap.end()) {
                    LOG_ERROR (m_componentArrayInfo.resource.logObj) << "Component does not exist"
                                                                     << " "
                                                                     << "[" << entity << "]"
                                                                     << std::endl;
                    throw std::runtime_error ("Component does not exist");
                }
                return &meta.array[meta.entityToIdxMap[entity]];
            }

            void onRemoveEntity (const Entity entity) override {
                auto& meta = m_componentArrayInfo.meta;
                if (meta.entityToIdxMap.find (entity) != meta.entityToIdxMap.end())
                    removeComponent (entity);
            }

            void onGenerateReport (void) override {
                auto& logObj       = m_componentArrayInfo.resource.logObj;
                auto& meta         = m_componentArrayInfo.meta;
                std::string spacer = "";

                LOG_LITE_INFO (logObj) << "[";
                for (Entity i = 0; i < g_maxEntities; i++) {
                    if (meta.entityToIdxMap.find (i) != meta.entityToIdxMap.end())
                        LOG_LITE_INFO (logObj) << spacer << "O";
                    else
                        LOG_LITE_INFO (logObj) << spacer << "X";
                    spacer = ", ";
                }
                LOG_LITE_INFO (logObj) << "]";
            }

            ~SNComponentArray (void) {
                if (m_componentArrayInfo.state.logObjCreated)
                    delete m_componentArrayInfo.resource.logObj;
            }
    };
}   // namespace Scene