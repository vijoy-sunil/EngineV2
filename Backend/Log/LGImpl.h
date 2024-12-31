#pragma once
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Layer/LYCommon.h"
#include "LGEnum.h"

namespace Log {
    class LGImpl: public Layer::NonTemplateBase {
        private:
            std::unordered_map <e_logLevel, e_logSink> m_logConfigs;
            std::string m_saveDirPath;
            std::string m_saveFileName;
            bool m_headerDisabled;

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
                m_logConfigs[LOG_LEVEL_INFO]    = LOG_SINK_CONSOLE;
                m_logConfigs[LOG_LEVEL_WARNING] = LOG_SINK_CONSOLE;
                m_logConfigs[LOG_LEVEL_ERROR]   = LOG_SINK_CONSOLE;

                m_saveDirPath                   = ".";
                m_saveFileName                  = "log.txt";
                m_headerDisabled                = headerDisabled;
            }

            void updateLogConfig (const e_logLevel level, const e_logSink sink) {
                m_logConfigs[level] = sink;
            }

            void updateSaveLocation (const std::string saveDirPath, const std::string saveFileName) {
                m_saveDirPath  = saveDirPath;
                m_saveFileName = saveFileName;
            }

            void writeToSink (const e_logLevel level,
                              const std::string message,
                              const char* functionName,
                              const int32_t lineNumber) {

                /* Filtering */
                if (m_logConfigs[level] == LOG_SINK_NONE)
                    return;

                std::string header     = getHeader (level, functionName, lineNumber);
                std::string logMessage = m_headerDisabled ? message : header + message;
                auto sink              = m_logConfigs[level];

                if (sink & LOG_SINK_CONSOLE)
                    std::cout << logMessage << std::endl;

                if (sink & LOG_SINK_FILE) {
                    std::ofstream file;
                    file.open (m_saveDirPath + "/" + m_saveFileName, std::ios::app);
                    file << logMessage << std::endl;
                    file.close();
                }
            }
    };
}   // namespace Log