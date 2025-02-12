#pragma once
#include <unordered_map>
#include <iomanip>
#include "CNTypeInstanceBase.h"
#include "CNTypeInstancePool.h"
#include "../Log/LGImpl.h"
#include "../Log/LGEnum.h"

namespace Collection {
    class CNImpl {
        private:
            struct CollectionInfo {
                struct Meta {
                    std::unordered_map <const char*, CNTypeInstancePool*> types;
                } meta;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_collectionInfo;

        public:
            CNImpl (void) {
                m_collectionInfo = {};

                m_collectionInfo.resource.logObj = new Log::LGImpl();
                m_collectionInfo.resource.logObj->initLogInfo     ("Build/Log/Collection", __FILE__);
                m_collectionInfo.resource.logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                m_collectionInfo.resource.logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE |
                                                                                           Log::LOG_SINK_FILE);
                m_collectionInfo.resource.logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE |
                                                                                           Log::LOG_SINK_FILE);
            }

            void initCollectionInfo (void) {
                m_collectionInfo.meta.types = {};
            }

            template <typename T>
            void registerCollectionType (void) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) != meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type already registered"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type already registered");
                }
                auto instancePoolObj = new CNTypeInstancePool (m_collectionInfo.resource.logObj);
                instancePoolObj->initTypeInstancePoolInfo();

                meta.types.insert ({typeName, instancePoolObj});
            }

            template <typename T>
            void addCollectionTypeInstance (const char* instanceId, CNTypeInstanceBase* instanceBaseObj) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instancePoolObj = meta.types[typeName];
                instancePoolObj->addCollectionTypeInstance (instanceId, instanceBaseObj);
                /* Run on attach */
                instanceBaseObj->onAttach();
            }

            template <typename T>
            void removeCollectionTypeInstance (const char* instanceId) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instancePoolObj = meta.types[typeName];
                auto instanceBaseObj = instancePoolObj->removeCollectionTypeInstance (instanceId);
                /* Run on detach */
                instanceBaseObj->onDetach();
                /* Note that, even though the instance object is allocated by the application, its memory is freed upon
                 * detach by the collection
                */
                delete instanceBaseObj;
            }

            template <typename T>
            T* getCollectionTypeInstance (const char* instanceId) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instancePoolObj = meta.types[typeName];
                auto instanceBaseObj = instancePoolObj->getCollectionTypeInstance (instanceId);

                return static_cast <T*> (instanceBaseObj);
            }

            template <typename T>
            void updateCollectionType (const float frameDelta) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                /* Run on update */
                auto instancePoolObj = meta.types[typeName];
                auto instancePool    = instancePoolObj->getCollectionTypeInstancePool();
                for (auto const& instanceBaseObj: instancePool)
                    instanceBaseObj->onUpdate (frameDelta);
            }

            void generateReport (void) {
                auto& logObj = m_collectionInfo.resource.logObj;

                LOG_LITE_INFO (logObj) << "{" << std::endl;
                for (auto const& [typeName, instancePoolObj]: m_collectionInfo.meta.types) {
                    LOG_LITE_INFO (logObj) << "\t";
                    LOG_LITE_INFO (logObj) << std::left << std::setw (15) << typeName << ", ";
                    instancePoolObj->generateReport();
                    LOG_LITE_INFO (logObj) << std::endl;
                }
                LOG_LITE_INFO (logObj) << "}" << std::endl;
            }

            ~CNImpl (void) {
                delete m_collectionInfo.resource.logObj;

                for (auto const& [typeName, instancePoolObj]: m_collectionInfo.meta.types)
                    delete instancePoolObj;
            }
    };
}   // namespace Collection