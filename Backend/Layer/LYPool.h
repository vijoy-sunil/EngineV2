#pragma once
#include <map>
#include <iostream>
#include <functional>
#include "LYEnum.h"

namespace Layer {
    /* We need to establish some relation between all the objects in order to store them in a single container. One way
     * is to use a common non template base class
    */
    class NonTemplateBase {
        public:
            /* The destructor is virtual for the base class because if you did not have a virtual destructor and through
             * the pointer to base class when you call destructor you end up calling base class destructor. In this case
             * you want polymorphism to work on your destructor as well, for example, through calling destructor on your
             * base class you want to end up calling destructor of your most derived class not JUST your base class
            */
            virtual ~NonTemplateBase (void) = 0;
    };
    /* Pure virtual destructors must be defined, which is against the pure virtual behaviour. The only difference between
     * virtual and pure virtual destructor is, that pure virtual destructor will make its base class abstract, hence you
     * cannot create object of that class (hence why we are doing it). We need an implementation here because if you
     * derive anything from base (upcasting) and then try to delete or destroy it, base's destructor will eventually be
     * called. Since it is pure and doesn't have an implementation, will cause compilation error
    */
    inline NonTemplateBase::~NonTemplateBase (void) {}

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
                    NonTemplateBase* instance;
                    /* Bindings */
                    std::function <void (void)> run;
                    std::function <void (void)> destroy;
                } resource;
            };
            std::map <int32_t, std::vector <InstanceInfo>> m_layerPool;

        public:
            ~LYPool (void) {
                e_layerCode code = PENDING_REQUEST;
                /* Note that, we are destroying all the layers (and thus the destroy bindings) in the destructor. This
                 * way, we are not leaking any memory when a pool is destroyed
                */
                for (auto const& [layerId, infos]: m_layerPool)
                    destroyLayer (layerId, code);
            }

            void runPool (void) {
                e_layerCode code = PENDING_REQUEST;
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
                        info.meta.id           = instanceId;
                        info.state.runDisabled = false;
                        info.resource.instance = nullptr;
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