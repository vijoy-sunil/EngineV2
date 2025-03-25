#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include "../Collection/CNTypeInstanceBase.h"

/* 1-line log macros */
#define LOG_LITE(obj, level)            obj->getReference()                                     \
                                            << obj->setCurrentSinks (level)
#define LOG(obj, level)                 obj->getReference()                                     \
                                            << obj->setCurrentSinks (level)                     \
                                            << "[" << obj->getTimeStamp()            << "]"     \
                                            << " "                                              \
                                            << "[" << obj->getLogLevelString (level) << "]"     \
                                            << " "                                              \
                                            << ALIGN_AND_PAD_L << __FUNCTION__                  \
                                            << " "                                              \
                                            << ALIGN_AND_PAD_S << __LINE__                      \
                                            << " "

#define LOG_LITE_INFO(obj)              LOG_LITE (obj, Log::LOG_LEVEL_INFO)
#define LOG_LITE_WARNING(obj)           LOG_LITE (obj, Log::LOG_LEVEL_WARNING)
#define LOG_LITE_ERROR(obj)             LOG_LITE (obj, Log::LOG_LEVEL_ERROR)

#define LOG_INFO(obj)                   LOG      (obj, Log::LOG_LEVEL_INFO)
#define LOG_WARNING(obj)                LOG      (obj, Log::LOG_LEVEL_WARNING)
#define LOG_ERROR(obj)                  LOG      (obj, Log::LOG_LEVEL_ERROR)

#define ALIGN_AND_PAD_S                 std::right << std::setw (8)
#define ALIGN_AND_PAD_M                 std::right << std::setw (32)
#define ALIGN_AND_PAD_L                 std::right << std::setw (64)
#define ALIGN_AND_PAD_C(padding)        std::right << std::setw (padding)

#define NULL_LOGOBJ_MSG                 "logObj = nullptr, creating a new one"
#define NULL_DEPOBJ_MSG                 "Invalid one or more dependency resources"
#define LINE_BREAK                      "|-----------------------------------------------------------------|"

namespace Log {
    typedef enum {
        LOG_LEVEL_INFO      = 1,
        LOG_LEVEL_WARNING   = 2,
        LOG_LEVEL_ERROR     = 4,
    } e_logLevel;

    typedef enum {
        LOG_SINK_NONE       = 0,
        LOG_SINK_CONSOLE    = 1,
        LOG_SINK_FILE       = 2
    } e_logSink;

    inline e_logSink operator | (const e_logSink sinkA, const e_logSink sinkB) {
        return static_cast <e_logSink> (static_cast <int> (sinkA) | static_cast <int> (sinkB));
    }

    class LGImpl: public Collection::CNTypeInstanceBase {
        private:
            struct LogInfo {
                struct Meta {
                    std::unordered_map <e_logLevel, e_logSink> configs;
                    const char* saveFileExtension;
                } meta;

                struct Path {
                    std::string saveDir;
                    std::string saveFileName;
                } path;

                struct Resource {
                    std::fstream file;
                } resource;
            } m_logInfo;

            e_logSink m_currentSinks;
            /* std::endl is a template function, and this is the signature of that function */
            using EndlType = std::ostream& (std::ostream&);

        public:
            LGImpl (void) {
                m_logInfo = {};
            }

            void initLogInfo (const std::string saveDirPath,
                              std::string saveFileName,
                              const char* saveFileExtension = ".log") {

                /* If we are using __FILE__ as save file name, strip path and extension to get just its name */
                size_t stripStart = saveFileName.find_last_of ("\\/") + 1;
                size_t stripEnd   = saveFileName.find_last_of ('.');
                saveFileName      = saveFileName.substr (stripStart, stripEnd - stripStart);

                /* Default log configs */
                m_logInfo.meta.configs[LOG_LEVEL_INFO]    = LOG_SINK_CONSOLE;
                m_logInfo.meta.configs[LOG_LEVEL_WARNING] = LOG_SINK_CONSOLE;
                m_logInfo.meta.configs[LOG_LEVEL_ERROR]   = LOG_SINK_CONSOLE;

                m_logInfo.meta.saveFileExtension          = saveFileExtension;
                m_logInfo.path.saveDir                    = saveDirPath;
                m_logInfo.path.saveFileName               = saveFileName;
                m_currentSinks                            = LOG_SINK_NONE;
            }

            void updateLogConfig (const e_logLevel level, const e_logSink sink) {
                m_logInfo.meta.configs[level] = sink;
            }

            LGImpl& getReference (void) {
                return *this;
            }

            const char* setCurrentSinks (const e_logLevel level) {
                m_currentSinks = m_logInfo.meta.configs[level];
                return "";
            }

            std::string getTimeStamp (void) {
                std::stringstream stream;
                auto now = std::chrono::system_clock::now();
                auto t_c = std::chrono::system_clock::to_time_t (now);
                /* https://en.cppreference.com/w/cpp/io/manip/put_time */
                stream << std::put_time (std::localtime (&t_c), "%F %T");
                return stream.str();
            }

            const char* getLogLevelString (const e_logLevel level) {
                switch (level) {
                    case LOG_LEVEL_INFO:        return "INFO";
                    case LOG_LEVEL_WARNING:     return "WARN";
                    case LOG_LEVEL_ERROR:       return "ERRO";
                    default:                    return "UNDF";
                }
            }
            /* We are using an explicit overload of operator << for std::endl only, and other templated for everything
             * else (https://stackoverflow.com/questions/17595957/operator-overloading-in-c-for-logging-purposes)
            */
            template <typename T>
            LGImpl& operator << (const T& data) {
                if (m_currentSinks & LOG_SINK_CONSOLE)
                    std::cout << data;

                if (m_currentSinks & LOG_SINK_FILE) {
                    m_logInfo.resource.file.open (m_logInfo.path.saveDir + "/" +
                                                  m_logInfo.path.saveFileName  +
                                                  m_logInfo.meta.saveFileExtension, std::ios::app);
                    m_logInfo.resource.file << data;
                    m_logInfo.resource.file.close();
                }
                return *this;
            }

            LGImpl& operator << (EndlType endl) {
                if (m_currentSinks & LOG_SINK_CONSOLE)
                    std::cout << endl;

                if (m_currentSinks & LOG_SINK_FILE) {
                    m_logInfo.resource.file.open (m_logInfo.path.saveDir + "/" +
                                                  m_logInfo.path.saveFileName  +
                                                  m_logInfo.meta.saveFileExtension, std::ios::app);
                    m_logInfo.resource.file << endl;
                    m_logInfo.resource.file.close();
                }
                return *this;
            }

            void onAttach (void) override {
                /* Do nothing */
            }

            void onDetach (void) override {
                /* Do nothing */
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }
    };
}   // namespace Log