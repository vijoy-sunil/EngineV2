#pragma once
#include "SNType.h"

namespace Scene {
    class SNComponentArrayBase {
        public:
            /* An interface is needed so that the component mgr can tell a generic component array, for example, that an
             * entity has been destroyed and that it needs to update its array mappings
            */
            virtual void onRemoveEntity   (const Entity entity) = 0;
            virtual void onGenerateReport (void) = 0;
            virtual ~SNComponentArrayBase (void) = 0;
    };
    inline SNComponentArrayBase::~SNComponentArrayBase (void) {}
}   // namespace Scene