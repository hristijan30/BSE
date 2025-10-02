#include "Logger.h"

namespace BSE
{
    std::ofstream Logger::s_logFile;
    std::vector<std::string> Logger::s_cache;
    std::mutex Logger::s_mutex;

    void Logger::Initialize(const std::string& filename)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_logFile.open(filename, std::ios::out | std::ios::app);
        if (!s_logFile.is_open())
        {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    void Logger::LogMessage(const std::string& text)
    {
        LogInternal(text, "");
    }

    void Logger::LogWarning(const std::string& text)
    {
        LogInternal(text, "WARNING: ");
    }

    void Logger::LogError(const std::string& text)
    {
        LogInternal(text, "ERROR: ");
    }

    void Logger::LogInternal(const std::string& text, const std::string& prefix)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        std::string fullText = prefix + text;

        s_cache.push_back(fullText);

        if (prefix == "ERROR: ")
            std::cerr << fullText << std::endl;
        else
            std::cout << fullText << std::endl;

        if (s_logFile.is_open())
        {
            s_logFile << fullText << std::endl;
            s_logFile.flush();
        }
    }

    const std::vector<std::string>& Logger::GetCachedLogs()
    {
        return s_cache;
    }
}
