#pragma once
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Layer/LYCommon.h"
#include "LGEnum.h"
/* Log macros */
#define LOG_INFO(message)   writeToSink (Log::LOG_LEVEL_INFO,    message, __FUNCTION__, __LINE__)
#define LOG_WARN(message)   writeToSink (Log::LOG_LEVEL_WARNING, message, __FUNCTION__, __LINE__)
#define LOG_ERRO(message)   writeToSink (Log::LOG_LEVEL_ERROR,   message, __FUNCTION__, __LINE__)

namespace Log {
    class LGImpl: public Layer::NonTemplateBase {
        private:
            struct LogInfo {
                struct Meta {
                    std::unordered_map <e_logLevel, e_logSink> logConfigs;
                } meta;

                struct State {
                    bool headerDisabled;
                } state;

                struct Path {
                    std::string saveDir;
                    std::string saveFileName;
                } path;
            } m_logInfo;

            LogInfo m_savedLogInfo;

            std::string getTimeStamp (void) {
                std::stringstream stream;
                auto now = std::chrono::system_clock::now();
                auto t_c = std::chrono::system_clock::to_time_t (now);
                /* https://en.cppreference.com/w/cpp/io/manip/put_time */
                stream << std::put_time (std::localtime (&t_c), "%F %T");
                return stream.str();
            }

            std::string getHeader (const e_logLevel level,
                                   const char* functionName,
                                   const int32_t lineNumber) {

                return "[" + getTimeStamp()              + "]" + " " +
                       "[" + getLogLevelString (level)   + "]" + " " +
                             functionName                      + " " +
                             std::to_string (lineNumber) + " ";
            }

        public:
            LGImpl (const bool headerDisabled = false) {
                /* Default log configs */
                m_logInfo.meta.logConfigs[LOG_LEVEL_INFO]    = LOG_SINK_CONSOLE;
                m_logInfo.meta.logConfigs[LOG_LEVEL_WARNING] = LOG_SINK_CONSOLE;
                m_logInfo.meta.logConfigs[LOG_LEVEL_ERROR]   = LOG_SINK_CONSOLE;

                m_logInfo.state.headerDisabled               = headerDisabled;
                m_logInfo.path.saveDir                       = ".";
                m_logInfo.path.saveFileName                  = "log.txt";
                m_savedLogInfo                               = m_logInfo;
            }

            void updateLogConfig (const e_logLevel level, const e_logSink sink) {
                m_logInfo.meta.logConfigs[level] = sink;
            }

            void updateSaveLocation (const std::string saveDirPath, const std::string saveFileName) {
                m_logInfo.path.saveDir      = saveDirPath;
                m_logInfo.path.saveFileName = saveFileName;
            }

            /* The save and restore methods can be used to temporarily change for example, the log configs in a specific
             * file or function by saving them first and then upon exiting the said file/function, restore the original
             * config data
            */
            void saveLogInfo (void) {
                m_savedLogInfo = m_logInfo;
            }

            void restoreLogInfo (void) {
                m_logInfo = m_savedLogInfo;
            }

            void writeToSink (const e_logLevel level,
                              const std::string message,
                              const char* functionName,
                              const int32_t lineNumber) {

                /* Filtering */
                if (m_logInfo.meta.logConfigs[level] == LOG_SINK_NONE)
                    return;

                std::string header     = getHeader (level, functionName, lineNumber);
                std::string logMessage = m_logInfo.state.headerDisabled ? message : header + message;
                auto sink              = m_logInfo.meta.logConfigs[level];

                if (sink & LOG_SINK_CONSOLE)
                    std::cout << logMessage << std::endl;

                if (sink & LOG_SINK_FILE) {
                    std::ofstream file;
                    file.open (m_logInfo.path.saveDir + "/" + m_logInfo.path.saveFileName, std::ios::app);
                    file << logMessage << std::endl;
                    file.close();
                }
            }
    };
}   // namespace Log