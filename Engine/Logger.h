#pragma once

#include "Define.h"
#include "StandardInclude.h"

namespace BSE
{
    class DLL_EXPORT Logger
    {
        Logger() = delete;

    public:
        static void Initialize(const std::string& filename);
        static void LogMessage(const std::string& text);
        static void LogWarning(const std::string& text);
        static void LogError(const std::string& text);

        static const std::vector<std::string>& GetCachedLogs();

    private:
        static void LogInternal(const std::string& text, const std::string& prefix);

        static std::ofstream s_logFile;
        static std::vector<std::string> s_cache;
        static std::mutex s_mutex;
    };
}
