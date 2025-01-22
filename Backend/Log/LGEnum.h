#pragma once

namespace Log {
    typedef enum {
        LOG_LEVEL_INFO      = 1,
        LOG_LEVEL_WARNING   = 2,
        LOG_LEVEL_ERROR     = 4,
    } e_logLevel;

    const char* getLogLevelString (const e_logLevel level) {
        switch (level) {
            case LOG_LEVEL_INFO:        return "INFO";
            case LOG_LEVEL_WARNING:     return "WARN";
            case LOG_LEVEL_ERROR:       return "ERRO";
            default:                    return "UNDF";
        }
    }

    typedef enum {
        LOG_SINK_NONE       = 0,
        LOG_SINK_CONSOLE    = 1,
        LOG_SINK_FILE       = 2
    } e_logSink;

    inline e_logSink operator | (const e_logSink sinkA, const e_logSink sinkB) {
        return static_cast <e_logSink> (static_cast <int> (sinkA) | static_cast <int> (sinkB));
    }
}   // namespace Log