#pragma once

namespace SandBox {
    /* TYPE-1: Press to lock/unlock
     *  +-------------------+           +-------------------+           +-------------------+
     *  |      _LOCKED      |---------->|  _PENDING_UNLOCK  |---------->|     _UNLOCKED     |
     *  +-------------------+   press   +-------------------+  release  +-------------------+
     *            ^                             hold                              :
     *            :                                                               :
     *            :                                                               v
     *  +-------------------+           +-------------------+           +-------------------+
     *  |      _LOCKED      |<----------|   _PENDING_LOCK   |<----------|     _UNLOCKED     |
     *  +-------------------+  release  +-------------------+   press   +-------------------+
     *                                          hold
     *
     *  TYPE-2: Hold to unlock
     *  +-------------------+           +-------------------+           +-------------------+
     *  |      _LOCKED      |---------->|     _UNLOCKED     |---------->|      _LOCKED      |
     *  +-------------------+   press   +-------------------+  release  +-------------------+
     *                                          hold
    */
    typedef enum {
        KEY_STATE_LOCKED,
        KEY_STATE_PENDING_UNLOCK,
        KEY_STATE_UNLOCKED,
        KEY_STATE_PENDING_LOCK
    } e_keyState;

    typedef enum {
        SENSITIVITY_TYPE_FINE,
        SENSITIVITY_TYPE_COARSE
    } e_sensitivityType;
}   // namespace SandBox