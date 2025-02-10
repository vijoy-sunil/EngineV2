#pragma once
#include <bitset>

namespace Scene {
    using Entity                            = uint32_t;
    using ComponentType                     = uint32_t;

    const Entity g_maxEntities              = 8;
    const ComponentType g_maxComponentTypes = 8;
    /* Since an entity is simply an id, we need a way to track which components an entity “has”, and we also need a way
     * to track which components a system cares about. Each component type has a unique id (starting from 0), which is
     * used to represent a bit in the signature. A system would register its interest in certain components as another
     * signature. Then it’s a simple bitwise comparison to ensure that an entity’s signature contains the system’s
     * signature (an entity might have more components than a system requires, which is fine, as long as it has all of
     * the components a system requires)
    */
    using Signature                         = std::bitset <g_maxComponentTypes>;
}   // namespace Scene