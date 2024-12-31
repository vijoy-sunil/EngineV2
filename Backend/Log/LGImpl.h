#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Layer/LYCommon.h"
#include "LGEnum.h"

namespace Log {
    class LGImpl: public Layer::NonTemplateBase {
        private:
            e_logLevel m_level;
            e_logSink m_sink;
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
            LGImpl (const e_logLevel level,
                    const e_logSink sink,
                    const std::string saveDirPath,
                    const std::string saveFileName,
                    const bool headerDisabled = false) {

                m_level          = level;
                m_sink           = sink;
                m_saveDirPath    = saveDirPath;
                m_saveFileName   = saveFileName;
                m_headerDisabled = headerDisabled;
            }

            void updateLevel (const e_logLevel level) {
                m_level = level;
            }

            void updateSink (const e_logSink sink) {
                m_sink = sink;
            }

            void writeToSink (const e_logLevel level,
                              const std::string message,
                              const char* functionName,
                              const int32_t lineNumber) {

                /* Level filtering */
                if ((m_level & level) == 0)
                    return;

                std::string header     = getHeader (level, functionName, lineNumber);
                std::string logMessage = m_headerDisabled ? message : header + message;

                /* Sink filtering */
                if (m_sink & LOG_SINK_CONSOLE)
                    std::cout << logMessage << std::endl;

                if (m_sink & LOG_SINK_FILE) {
                    std::ofstream file;
                    file.open (m_saveDirPath + "/" + m_saveFileName, std::ios::app);
                    file << logMessage << std::endl;
                    file.close();
                }
            }
    };
}   // namespace Log