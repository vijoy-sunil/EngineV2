#pragma once
#include <map>
#include <iostream>
#include <functional>
#include "LYCommon.h"
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
                    NonTemplateBase* instanceObj;
                    /* Bindings */
                    std::function <void (void)> run;
                    std::function <void (void)> destroy;
                } resource;
            };
            std::map <int32_t, std::vector <InstanceInfo>> m_layerPool;

        public:
            ~LYPool (void) {
                auto code = PENDING_REQUEST;
                /* Note that, we are destroying all the layers (and thus executing the destroy bindings) in the
                 * destructor. This way, we are not leaking any memory when a pool is destroyed
                */
                for (auto it = m_layerPool.begin(); it != m_layerPool.end();) {
                    auto itToErase = it;
                    ++it;
                    destroyLayer (itToErase->first, code);
                }
            }

            void runPool (void) {
                auto code = PENDING_REQUEST;
                /* Note that, not having an unordered map for the pool is essential if you want to run the layers in
                 * the order they were created
                */
                for (auto const& [layerId, infos]: m_layerPool)
                    runLayer (layerId, code);
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

            void runLayer (const int32_t layerId, e_layerCode& code) {
                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    code = VALID_REQUEST;
                    for (auto const& info: m_layerPool[layerId]) {
                        if (!(info.state.runDisabled) && (info.resource.run != nullptr))
                            info.resource.run();
                    }
                }
                else
                    code = LAYER_DOES_NOT_EXIST;
            }

            void destroyLayer (const int32_t layerId, e_layerCode& code) {
                if (m_layerPool.find (layerId) != m_layerPool.end()) {
                    code = VALID_REQUEST;
                    for (auto const& info: m_layerPool[layerId]) {
                        if (info.resource.destroy != nullptr)
                            info.resource.destroy();
                    }
                    m_layerPool.erase (layerId);
                }
                else
                    code = LAYER_DOES_NOT_EXIST;
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
                        info.meta.id              = instanceId;
                        info.state.runDisabled    = false;
                        info.resource.instanceObj = nullptr;
                        info.resource.run         = nullptr;
                        info.resource.destroy     = nullptr;

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
                            it->resource.destroy();
                        }
                        else
                            code = LAYER_INSTANCE_MISSING_BINDING;
                        infos.erase (it);
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