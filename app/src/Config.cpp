#include "Config.hpp"

#include <fstream>
#include <filesystem>
#include <fmt/format.h>

using namespace std;

namespace
{
    std::string ReadWholeFile(const char* path)
    {
        std::ifstream t(path);
        if (!t.is_open())
            throw std::runtime_error(fmt::format("Failed to open file: {}", path));
        t.seekg(0, std::ios::end);
        size_t size = t.tellg();
        std::string buffer(size, ' ');
        t.seekg(0);
        t.read(&buffer[0], size);
        if (!t)
            throw std::runtime_error(fmt::format("Failed to read file: {}", path));
        return buffer;
    }
}

void ProfileDesc::LoadFromJson(const nlohmann::json& json)
{
    if (json.contains("stapm_limit") && json["stapm_limit"].is_number())
        StapmLimit = json["stapm_limit"].get<float>();
    else
        StapmLimit = std::numeric_limits<float>::quiet_NaN();

    if (json.contains("ppt_limit_fast") && json["ppt_limit_fast"].is_number())
        PptLimitFast = json["ppt_limit_fast"].get<float>();
    else
        PptLimitFast = std::numeric_limits<float>::quiet_NaN();

    if (json.contains("ppt_limit_slow") && json["ppt_limit_slow"].is_number())
        PptLimitSlow = json["ppt_limit_slow"].get<float>();
    else
        PptLimitSlow = std::numeric_limits<float>::quiet_NaN();
}

void Config::LoadFromJson(const nlohmann::json& json)
{
    Profiles.clear();
    if (json.contains("profiles") && json["profiles"].is_object())
    {
        for (auto& item : json["profiles"].items())
        {
            auto& profileName = item.key();
            auto& profileJson = item.value();
            ProfileDesc profile;
            profile.LoadFromJson(profileJson);
            Profiles[profileName] = profile;
        }
    }

    if (json.contains("refresh_interval_ms") && json["refresh_interval_ms"].is_number())
        RefreshIntervalMs = json["refresh_interval_ms"].get<int>();
    else
        RefreshIntervalMs = 3000; // Default value
}

void Config::LoadFromFile()
{
    if (std::filesystem::exists("config.json"))
    {
        auto jsonStr = ReadWholeFile("config.json");
        auto json = nlohmann::json::parse(jsonStr);
        LoadFromJson(json);
    }
    else
    {
        throw std::runtime_error("Config file not found");
    }
}
