#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"

/* 1-line log macros */
#define LOG_LITE(obj, level)            obj->getReference()                                     \
                                            << obj->updateActiveSinkType (level)
#define LOG(obj, level)                 obj->getReference()                                     \
                                            << obj->updateActiveSinkType (level)                \
                                            << "[" << obj->getTimeStamp()             << "]"    \
                                            << " "                                              \
                                            << "[" << obj->getLevelTypeString (level) << "]"    \
                                            << " "                                              \
                                            << ALIGN_AND_PAD_L << __FUNCTION__                  \
                                            << " "                                              \
                                            << ALIGN_AND_PAD_S << __LINE__                      \
                                            << " "

#define LOG_LITE_INFO(obj)              LOG_LITE (obj, Log::LEVEL_TYPE_INFO)
#define LOG_LITE_WARNING(obj)           LOG_LITE (obj, Log::LEVEL_TYPE_WARNING)
#define LOG_LITE_ERROR(obj)             LOG_LITE (obj, Log::LEVEL_TYPE_ERROR)

#define LOG_INFO(obj)                   LOG      (obj, Log::LEVEL_TYPE_INFO)
#define LOG_WARNING(obj)                LOG      (obj, Log::LEVEL_TYPE_WARNING)
#define LOG_ERROR(obj)                  LOG      (obj, Log::LEVEL_TYPE_ERROR)

#define ALIGN_AND_PAD_S                 std::right << std::setw (8)
#define ALIGN_AND_PAD_M                 std::right << std::setw (32)
#define ALIGN_AND_PAD_L                 std::right << std::setw (64)
#define ALIGN_AND_PAD_C(padding)        std::right << std::setw (padding)

#define NULL_LOGOBJ_MSG                 "logObj = nullptr, creating a new one"
#define NULL_DEPOBJ_MSG                 "Invalid one or more dependency resources"
#define LINE_BREAK                      "|-----------------------------------------------------------------|"

namespace Log {
    typedef enum {
        LEVEL_TYPE_INFO    = 1,
        LEVEL_TYPE_WARNING = 2,
        LEVEL_TYPE_ERROR   = 4,
    } e_levelType;

    typedef enum {
        SINK_TYPE_NONE     = 0,
        SINK_TYPE_CONSOLE  = 1,
        SINK_TYPE_FILE     = 2
    } e_sinkType;

    inline e_sinkType operator | (const e_sinkType sinkTypeA, const e_sinkType sinkTypeB) {
        return static_cast <e_sinkType> (static_cast <int> (sinkTypeA) | static_cast <int> (sinkTypeB));
    }

    class LGImpl: public Collection::CNTypeInstanceBase {
        private:
            struct LogInfo {
                struct Meta {
                    std::unordered_map <e_levelType, e_sinkType> levelTypeToSinkTypeMap;
                    std::string saveFileDirPath;
                    std::string saveFileName;
                    const char* saveFileExtension;
                } meta;

                struct State {
                    e_sinkType activeSinkType;
                } state;

                struct Resource {
                    std::fstream file;
                } resource;
            } m_logInfo;

            /* std::endl is a template function, and this is the signature of that function */
            using EndlType = std::ostream& (std::ostream&);

        public:
            LGImpl (void) {
                m_logInfo = {};
            }

            void initLogInfo (const std::string saveFileDirPath,
                              std::string saveFileName,
                              const char* saveFileExtension = ".log") {

                auto& meta        = m_logInfo.meta;
                /* If we are using __FILE__ as save file name, strip path and extension to get just its name */
                size_t stripStart = saveFileName.find_last_of ("\\/") + 1;
                size_t stripEnd   = saveFileName.find_last_of ('.');
                saveFileName      = saveFileName.substr (stripStart, stripEnd - stripStart);

                /* Default log configs */
                meta.levelTypeToSinkTypeMap[LEVEL_TYPE_INFO]    = SINK_TYPE_CONSOLE;
                meta.levelTypeToSinkTypeMap[LEVEL_TYPE_WARNING] = SINK_TYPE_CONSOLE;
                meta.levelTypeToSinkTypeMap[LEVEL_TYPE_ERROR]   = SINK_TYPE_CONSOLE;

                meta.saveFileDirPath                            = saveFileDirPath;
                meta.saveFileName                               = saveFileName;
                meta.saveFileExtension                          = saveFileExtension;
                m_logInfo.state.activeSinkType                  = SINK_TYPE_NONE;
            }

            LGImpl& getReference (void) {
                return *this;
            }

            void updateLogConfig (const e_levelType levelType, const e_sinkType sinkType) {
                m_logInfo.meta.levelTypeToSinkTypeMap[levelType] = sinkType;
            }

            const char* updateActiveSinkType (const e_levelType levelType) {
                m_logInfo.state.activeSinkType = m_logInfo.meta.levelTypeToSinkTypeMap[levelType];
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

            const char* getLevelTypeString (const e_levelType levelType) {
                switch (levelType) {
                    case LEVEL_TYPE_INFO:       return "INFO";
                    case LEVEL_TYPE_WARNING:    return "WARN";
                    case LEVEL_TYPE_ERROR:      return "ERRO";
                    default:                    return "UNDF";
                }
            }
            /* We are using an explicit overload of operator << for std::endl only, and other templated for everything
             * else (https://stackoverflow.com/questions/17595957/operator-overloading-in-c-for-logging-purposes)
            */
            template <typename T>
            LGImpl& operator << (const T& data) {
                auto& meta           = m_logInfo.meta;
                auto& activeSinkType = m_logInfo.state.activeSinkType;
                auto& file           = m_logInfo.resource.file;

                if (activeSinkType & SINK_TYPE_CONSOLE)
                    std::cout << data;

                if (activeSinkType & SINK_TYPE_FILE) {
                    file.open (meta.saveFileDirPath + "/" +
                               meta.saveFileName    +
                               meta.saveFileExtension, std::ios::app);
                    file << data;
                    file.close();
                }
                return *this;
            }

            LGImpl& operator << (EndlType endl) {
                auto& meta           = m_logInfo.meta;
                auto& activeSinkType = m_logInfo.state.activeSinkType;
                auto& file           = m_logInfo.resource.file;

                if (activeSinkType & SINK_TYPE_CONSOLE)
                    std::cout << endl;

                if (activeSinkType & SINK_TYPE_FILE) {
                    file.open (meta.saveFileDirPath + "/" +
                               meta.saveFileName    +
                               meta.saveFileExtension, std::ios::app);
                    file << endl;
                    file.close();
                }
                return *this;
            }

            void onAttach (void) override {
                /* Do nothing */
            }

            void onDetach (void) override {
                /* Do nothing */
            }

            void onUpdate (void) override {
                /* Do nothing */
            }
    };
}   // namespace Log