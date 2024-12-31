#pragma once

/* Log macros */
#define LOG_INFO(message)   writeToSink (Log::LOG_LEVEL_INFO,    message, __FUNCTION__, __LINE__)
#define LOG_WARN(message)   writeToSink (Log::LOG_LEVEL_WARNING, message, __FUNCTION__, __LINE__)
#define LOG_ERRO(message)   writeToSink (Log::LOG_LEVEL_ERROR,   message, __FUNCTION__, __LINE__)