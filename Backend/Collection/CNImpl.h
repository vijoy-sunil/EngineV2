#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "CNTypeInstanceBase.h"
#include "CNTypeInstanceArray.h"
#include "../Log/LGImpl.h"

namespace Collection {
    class CNImpl {
        private:
            struct CollectionInfo {
                struct Meta {
                    std::unordered_map <const char*, CNTypeInstanceArray*> types;
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
                auto instanceArrayObj = new CNTypeInstanceArray (m_collectionInfo.resource.logObj);
                instanceArrayObj->initTypeInstanceArrayInfo();

                meta.types.insert ({typeName, instanceArrayObj});
            }

            template <typename T>
            void addCollectionTypeInstance (const std::string instanceId, CNTypeInstanceBase* instanceBaseObj) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = meta.types[typeName];
                instanceArrayObj->addCollectionTypeInstance (instanceId, instanceBaseObj);
                /* Run on attach */
                instanceBaseObj->onAttach();
            }

            template <typename T>
            void removeCollectionTypeInstance (const std::string instanceId) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = meta.types[typeName];
                auto instanceBaseObj  = instanceArrayObj->removeCollectionTypeInstance (instanceId);
                /* Run on detach */
                instanceBaseObj->onDetach();
                /* Note that, even though the instance object is allocated by the application, its memory is freed upon
                 * detach by the collection
                */
                delete instanceBaseObj;
            }

            template <typename T>
            T* getCollectionTypeInstance (const std::string instanceId) {
                const char* typeName = typeid (T).name();
                auto& meta           = m_collectionInfo.meta;
                if (meta.types.find (typeName) == meta.types.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = meta.types[typeName];
                auto instanceBaseObj  = instanceArrayObj->getCollectionTypeInstance (instanceId);

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
                auto instanceArrayObj = meta.types[typeName];
                auto instanceArray    = instanceArrayObj->getCollectionTypeInstanceArray();
                for (auto const& instanceBaseObj: instanceArray) {
                    if (instanceBaseObj != nullptr)
                        instanceBaseObj->onUpdate (frameDelta);
                }
            }

            void generateReport (void) {
                auto& logObj = m_collectionInfo.resource.logObj;

                LOG_LITE_INFO (logObj)     << "{"      << std::endl;
                for (auto const& [typeName, instanceArrayObj]: m_collectionInfo.meta.types) {
                    LOG_LITE_INFO (logObj) << "\t";
                    LOG_LITE_INFO (logObj) << typeName << std::endl;
                    instanceArrayObj->generateReport();
                }
                LOG_LITE_INFO (logObj)     << "}"      << std::endl;
            }

            ~CNImpl (void) {
                delete m_collectionInfo.resource.logObj;

                for (auto const& [typeName, instanceArrayObj]: m_collectionInfo.meta.types)
                    delete instanceArrayObj;
            }
    };
}   // namespace Collection