#pragma once
#include "../Common.h"
#include "SNType.h"

namespace Scene {
    /* A system is any functionality that iterates upon a list of entities with a certain signature of components.
     * Every system needs a list of entities, and we want some logic outside of the system (in the form of a manager)
     * to maintain that list. Each system can then inherit from this class which allows the manager to keep a list of
     * pointers to systems
    */
    class SNSystemBase {
        public:
            std::set <Entity> m_entities;

            virtual ~SNSystemBase (void) = 0;
    };
    inline SNSystemBase::~SNSystemBase (void) {}
}   // namespace Scene