#pragma once
#include <map>
#include <iostream>
#include <functional>
#include "LYInstanceBase.h"
#include "LYEnum.h"

namespace Layer {
    class LYPool {
        private:
            struct InstanceInfo {
                struct Meta {
                    std::string id;
                } meta;

                struct State {
                    bool runDisabled;
                } state;

                struct Resource {
                    LYInstanceBase* obj;
                    /* Bindings */
                    std::function <void (void)> run;
                    std::function <void (void)> destroy;
                } resource;
            };
            std::map <int32_t, std::vector <InstanceInfo>> m_layerPool;

        public:
            void runAllLayers (void) {
                auto codes = std::vector <e_layerCode> {};
                for (auto const& [layerId, infos]: m_layerPool)
                    runLayer (layerId, codes);
            }

            /* 3-step destroy */
            void destroyAllLayers (void) {
                for (auto it = m_layerPool.rbegin(); it != m_layerPool.rend(); it++) {
                    for (auto const& info: it->second) {
                        if (info.resource.destroy != nullptr)
                            info.resource.destroy();            /* (1) */

                        delete info.resource.obj;               /* (2) */
                    }
                }
                /* It is less complicated to clear the whole map at once, instead of individual layers since we do not
                 * have to worry about invalidating the iterator after each erase call
                */
                m_layerPool.clear();                            /* (3) */
            }

            void createLayer (const int32_t layerId, e_layerCode& code) {
                if (m_layerPool.find (layerId) != m_layerPool.end())
                    code = LAYER_ALREADY_EXISTS;
                else {
                    code = VALID_REQUEST;
                    m_layerPool.insert ({layerId, {}});
                }
            }

            std::vector <InstanceInfo> getLayer (const int32_t layerId, e_layerCode& code) {
                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    code = VALID_REQUEST;
                    return m_layerPool[layerId];
                }
                else {
                    code = LAYER_DOES_NOT_EXIST;
                    return {};
                }
            }

            void runLayer (const int32_t layerId, std::vector <e_layerCode>& codes) {
                auto code = PENDING_REQUEST;

                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    for (auto const& info: m_layerPool[layerId]) {
                        std::string instanceId = info.meta.id;
                        runLayerInstance (layerId, instanceId, code);
                        codes.push_back  (code);
                    }
                }
                else {
                    code = LAYER_DOES_NOT_EXIST;
                    codes.push_back (code);
                }
            }

            /* 4-step destroy */
            void destroyLayer (const int32_t layerId, std::vector <e_layerCode>& codes) {
                auto code = PENDING_REQUEST;

                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    for (auto const& info: m_layerPool[layerId]) {
                        std::string instanceId = info.meta.id;
                        destroyLayerInstance (layerId, instanceId, code);   /* (1), (2), (3) */
                        codes.push_back      (code);
                    }
                    m_layerPool.erase (layerId);                            /* (4) */
                }
                else {
                    code = LAYER_DOES_NOT_EXIST;
                    codes.push_back (code);
                }
            }

            void createLayerInstance (const int32_t layerId,
                                      const std::string instanceId,
                                      e_layerCode& code) {

                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    auto& infos = m_layerPool[layerId];
                    auto it     = std::find_if (infos.begin(), infos.end(),
                                  [instanceId] (const InstanceInfo& info) {
                                        return info.meta.id == instanceId;
                                  });

                    if (it != infos.end())
                        code = LAYER_INSTANCE_ALREADY_EXISTS;
                    else {
                        code = VALID_REQUEST;
                        InstanceInfo info;
                        info.meta.id           = instanceId;
                        info.state.runDisabled = false;
                        info.resource.obj      = nullptr;
                        info.resource.run      = nullptr;
                        info.resource.destroy  = nullptr;

                        infos.push_back (info);
                    }
                }
                else
                    code = LAYER_DOES_NOT_EXIST;
            }

            InstanceInfo* getLayerInstance (const int32_t layerId,
                                            const std::string instanceId,
                                            e_layerCode& code) {

                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    auto& infos = m_layerPool[layerId];
                    auto it     = std::find_if (infos.begin(), infos.end(),
                                  [instanceId] (const InstanceInfo& info) {
                                        return info.meta.id == instanceId;
                                  });

                    if (it != infos.end()) {
                        code = VALID_REQUEST;
                        return &(*it);
                    }
                    else {
                        code = LAYER_INSTANCE_DOES_NOT_EXIST;
                        return nullptr;
                    }
                }
                else {
                    code = LAYER_DOES_NOT_EXIST;
                    return nullptr;
                }
            }

            void runLayerInstance (const int32_t layerId,
                                   const std::string instanceId,
                                   e_layerCode& code) {

                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    auto const& infos = m_layerPool[layerId];
                    auto it           = std::find_if (infos.begin(), infos.end(),
                                        [instanceId] (const InstanceInfo& info) {
                                            return info.meta.id == instanceId;
                                        });

                    if (it != infos.end()) {
                        if (!(it->state.runDisabled) && (it->resource.run != nullptr)) {
                            code = VALID_REQUEST;
                            it->resource.run();
                        }
                        else if (it->state.runDisabled)
                            code = LAYER_INSTANCE_RUN_DISABLED;
                        else
                            code = LAYER_INSTANCE_MISSING_BINDING;
                    }
                    else
                        code = LAYER_INSTANCE_DOES_NOT_EXIST;
                }
                else
                    code = LAYER_DOES_NOT_EXIST;
            }

            /* 3-step destroy */
            void destroyLayerInstance (const int32_t layerId,
                                       const std::string instanceId,
                                       e_layerCode& code) {

                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    auto& infos = m_layerPool[layerId];
                    auto it     = std::find_if (infos.begin(), infos.end(),
                                  [instanceId] (const InstanceInfo& info) {
                                        return info.meta.id == instanceId;
                                  });

                    if (it != infos.end()) {
                        if (it->resource.destroy != nullptr) {
                            code = VALID_REQUEST;
                            it->resource.destroy();                 /* (1) */
                        }
                        else
                            code = LAYER_INSTANCE_MISSING_BINDING;

                        delete it->resource.obj;                    /* (2) */
                        infos.erase (it);                           /* (3) */
                    }
                    else
                        code = LAYER_INSTANCE_DOES_NOT_EXIST;
                }
                else
                    code = LAYER_DOES_NOT_EXIST;
            }

            void generateReport (void) {
                std::cout << "Layer pool report"         << std::endl;
                std::cout << "{"                         << std::endl;
                for (auto const& [layerId, infos]: m_layerPool) {
                    std::cout << "\t";
                    std::cout << "Layer id: " << layerId << std::endl;
                    std::cout << "\t";
                    std::cout << "{"                     << std::endl;

                    for (auto const& info: infos) {
                        std::string checkBoxGraphic = info.state.runDisabled ? "[X]" : "[O]";
                        std::cout << "\t\t";
                        std::cout << checkBoxGraphic     << " "
                                  << info.meta.id        << std::endl;
                    }
                    std::cout << "\t";
                    std::cout << "}"                     << std::endl;
                }
                std::cout << "}"                         << std::endl;
            }
    };
}   // namespace Layer