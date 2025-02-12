#pragma once

namespace Collection {
    class CNTypeInstanceBase {
        public:
            virtual void onAttach       (void)                   = 0;
            virtual void onDetach       (void)                   = 0;
            virtual void onUpdate       (const float frameDelta) = 0;
            virtual ~CNTypeInstanceBase (void)                   = 0;
    };
    inline CNTypeInstanceBase::~CNTypeInstanceBase (void) {}
}   // namespace Collection