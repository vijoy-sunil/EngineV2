#pragma once
#include "../Common.h"
#include "CNTypeInstanceBase.h"
#include "CNTypeInstanceArray.h"
#include "../Log/LGImpl.h"

namespace Collection {
    class CNImpl {
        private:
            struct CollectionInfo {
                struct Meta {
                    std::unordered_map <const char*, CNTypeInstanceArray*> typeNameToInstanceArrayObjMap;
                } meta;

                struct Resource {
                    Log::LGImpl* logObj;
                } resource;
            } m_collectionInfo;

        public:
            CNImpl (void) {
                m_collectionInfo = {};

                auto& logObj = m_collectionInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/Collection",  __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initCollectionInfo (void) {
                m_collectionInfo.meta.typeNameToInstanceArrayObjMap = {};
            }

            template <typename T>
            void registerCollectionType (void) {
                const char* typeName                = typeid (T).name();
                auto& typeNameToInstanceArrayObjMap = m_collectionInfo.meta.typeNameToInstanceArrayObjMap;
                auto& logObj                        = m_collectionInfo.resource.logObj;

                if (typeNameToInstanceArrayObjMap.find (typeName) != typeNameToInstanceArrayObjMap.end()) {
                    LOG_ERROR (logObj) << "Collection type already registered"
                                       << " "
                                       << "[" << typeName << "]"
                                       << std::endl;
                    throw std::runtime_error ("Collection type already registered");
                }
                auto instanceArrayObj = new CNTypeInstanceArray (logObj);
                instanceArrayObj->initTypeInstanceArrayInfo();

                typeNameToInstanceArrayObjMap.insert ({typeName, instanceArrayObj});
            }

            template <typename T>
            void addCollectionTypeInstance (const std::string instanceId, CNTypeInstanceBase* instanceBaseObj) {
                const char* typeName                = typeid (T).name();
                auto& typeNameToInstanceArrayObjMap = m_collectionInfo.meta.typeNameToInstanceArrayObjMap;

                if (typeNameToInstanceArrayObjMap.find (typeName) == typeNameToInstanceArrayObjMap.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = typeNameToInstanceArrayObjMap[typeName];
                instanceArrayObj->addCollectionTypeInstance (instanceId, instanceBaseObj);
                /* Run on attach */
                instanceBaseObj->onAttach();
            }

            template <typename T>
            void removeCollectionTypeInstance (const std::string instanceId) {
                const char* typeName                = typeid (T).name();
                auto& typeNameToInstanceArrayObjMap = m_collectionInfo.meta.typeNameToInstanceArrayObjMap;

                if (typeNameToInstanceArrayObjMap.find (typeName) == typeNameToInstanceArrayObjMap.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = typeNameToInstanceArrayObjMap[typeName];
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
                const char* typeName                = typeid (T).name();
                auto& typeNameToInstanceArrayObjMap = m_collectionInfo.meta.typeNameToInstanceArrayObjMap;

                if (typeNameToInstanceArrayObjMap.find (typeName) == typeNameToInstanceArrayObjMap.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = typeNameToInstanceArrayObjMap[typeName];
                auto instanceBaseObj  = instanceArrayObj->getCollectionTypeInstance (instanceId);

                return static_cast <T*> (instanceBaseObj);
            }

            template <typename T>
            void updateCollectionType (void) {
                const char* typeName                = typeid (T).name();
                auto& typeNameToInstanceArrayObjMap = m_collectionInfo.meta.typeNameToInstanceArrayObjMap;

                if (typeNameToInstanceArrayObjMap.find (typeName) == typeNameToInstanceArrayObjMap.end()) {
                    LOG_ERROR (m_collectionInfo.resource.logObj) << "Collection type not registered before use"
                                                                 << " "
                                                                 << "[" << typeName << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("Collection type not registered before use");
                }
                auto instanceArrayObj = typeNameToInstanceArrayObjMap[typeName];
                auto instanceArray    = instanceArrayObj->getCollectionTypeInstanceArray();
                /* Run on update */
                for (auto const& instanceBaseObj: instanceArray) {
                    if (instanceBaseObj != nullptr)
                        instanceBaseObj->onUpdate();
                }
            }

            void generateReport (void) {
                auto& logObj = m_collectionInfo.resource.logObj;

                LOG_LITE_INFO (logObj)     << "{"      << std::endl;
                for (auto const& [typeName, instanceArrayObj]: m_collectionInfo.meta.typeNameToInstanceArrayObjMap) {
                    LOG_LITE_INFO (logObj) << "\t";
                    LOG_LITE_INFO (logObj) << typeName << std::endl;
                    instanceArrayObj->generateReport();
                }
                LOG_LITE_INFO (logObj)     << "}"      << std::endl;
            }

            ~CNImpl (void) {
                delete m_collectionInfo.resource.logObj;

                for (auto const& [typeName, instanceArrayObj]: m_collectionInfo.meta.typeNameToInstanceArrayObjMap)
                    delete instanceArrayObj;
            }
    };
}   // namespace Collection