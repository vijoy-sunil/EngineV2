#pragma once
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Layer/LYInstanceBase.h"
#include "LGEnum.h"

/* 1-line log macros */
#define LOG_LITE(obj, level)            obj->getReference()                                     \
                                            << obj->setCurrentSinks (level)
#define LOG(obj, level)                 obj->getReference()                                     \
                                            << obj->setCurrentSinks (level)                     \
                                            << "[" << obj->getTimeStamp()       << "]"          \
                                            << " "                                              \
                                            << "[" << getLogLevelString (level) << "]"          \
                                            << " "                                              \
                                            << std::left << std::setw (35) << __FUNCTION__      \
                                            << " "                                              \
                                            << std::left << std::setw (4)  << __LINE__          \
                                            << " "

#define LOG_LITE_INFO(obj)              LOG_LITE (obj, Log::LOG_LEVEL_INFO)
#define LOG_LITE_WARNING(obj)           LOG_LITE (obj, Log::LOG_LEVEL_WARNING)
#define LOG_LITE_ERROR(obj)             LOG_LITE (obj, Log::LOG_LEVEL_ERROR)

#define LOG_INFO(obj)                   LOG      (obj, Log::LOG_LEVEL_INFO)
#define LOG_WARNING(obj)                LOG      (obj, Log::LOG_LEVEL_WARNING)
#define LOG_ERROR(obj)                  LOG      (obj, Log::LOG_LEVEL_ERROR)

#define NULL_LOGOBJ_MSG                 "logObj = nullptr, creating a new one"
#define NULL_DEPOBJ_MSG                 "Dependencies = nullptr"
#define LINE_BREAK                      "|-----------------------------------------------------------------|"

namespace Log {
    class LGImpl: public Layer::LYInstanceBase {
        private:
            struct LogInfo {
                struct Meta {
                    std::unordered_map <e_logLevel, e_logSink> configs;
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
            LogInfo   m_savedLogInfo;
            /* std::endl is a template function, and this is the signature of that function */
            using endl_type = std::ostream& (std::ostream&);

        public:
            LGImpl (const std::string saveDirPath  = ".",
                    const std::string saveFileName = "log.txt") {

                /* Default log configs */
                m_logInfo.meta.configs[LOG_LEVEL_INFO]    = LOG_SINK_CONSOLE;
                m_logInfo.meta.configs[LOG_LEVEL_WARNING] = LOG_SINK_CONSOLE;
                m_logInfo.meta.configs[LOG_LEVEL_ERROR]   = LOG_SINK_CONSOLE;

                m_logInfo.path.saveDir                    = saveDirPath;
                m_logInfo.path.saveFileName               = saveFileName;

                m_logInfo.resource.file.open (m_logInfo.path.saveDir + "/" +
                                              m_logInfo.path.saveFileName, std::ios::app);

                m_currentSinks                            = LOG_SINK_NONE;
                saveLogInfo();
            }

            /* The save and restore methods can be used to temporarily change for example, the log configs in a specific
             * file or function by saving them first, updating the configs and then upon exiting the said file/function,
             * restore the original config data
            */
            void saveLogInfo (void) {
                /* Copying of any stream in C++ is disabled by having made the copy constructor private. Any means any,
                 * whether it is stringstream, istream, ostream, iostream or whatever. Copying of stream is disabled
                 * because it doesn't make sense. A stream is not a container that you can make copy of. It doesn't
                 * contain data. If a list/vector/map or any container is a bucket, then stream is a hose through which
                 * data flows
                */
                m_savedLogInfo.meta = m_logInfo.meta;
                m_savedLogInfo.path = m_logInfo.path;
            }

            void updateLogConfig (const e_logLevel level, const e_logSink sink) {
                m_logInfo.meta.configs[level] = sink;
            }

            void updateSaveLocation (const std::string saveDirPath, const std::string saveFileName) {
                m_logInfo.path.saveDir      = saveDirPath;
                m_logInfo.path.saveFileName = saveFileName;
            }

            void restoreLogInfo (void) {
                m_logInfo.meta = m_savedLogInfo.meta;
                m_logInfo.path = m_savedLogInfo.path;
            }

            std::string getTimeStamp (void) {
                std::stringstream stream;
                auto now = std::chrono::system_clock::now();
                auto t_c = std::chrono::system_clock::to_time_t (now);
                /* https://en.cppreference.com/w/cpp/io/manip/put_time */
                stream << std::put_time (std::localtime (&t_c), "%F %T");
                return stream.str();
            }

            LGImpl& getReference (void) {
                return *this;
            }

            const char* setCurrentSinks (const e_logLevel level) {
                m_currentSinks = m_logInfo.meta.configs[level];
                return "";
            }

            /* We are using an explicit overload of operator << for std::endl only, and other templated for everything
             * else (https://stackoverflow.com/questions/17595957/operator-overloading-in-c-for-logging-purposes)
            */
            template <typename T>
            LGImpl& operator << (const T& data) {
                if (m_currentSinks & LOG_SINK_CONSOLE)
                    std::cout << data;

                if (m_currentSinks & LOG_SINK_FILE)
                    m_logInfo.resource.file << data;
                return *this;
            }

            LGImpl& operator << (endl_type endl){
                if (m_currentSinks & LOG_SINK_CONSOLE)
                    std::cout << endl;

                if (m_currentSinks & LOG_SINK_FILE)
                    m_logInfo.resource.file << endl;
                return *this;
            }

            void destroyLog (void) {
                std::string filePath = m_logInfo.path.saveDir + "/" +
                                       m_logInfo.path.saveFileName;
                /* Close and reopen file without any flags. Note that, if the flag is not set to any value, the initial
                 * position is the beginning of the file
                */
                m_logInfo.resource.file.close();
                m_logInfo.resource.file.open (filePath);
                /* Check if file is empty */
                if (m_logInfo.resource.file.peek() == std::ifstream::traits_type::eof())
                    remove (filePath.c_str());

                m_logInfo.resource.file.close();
            }
    };
}   // namespace Log