#pragma once
#include <limits>
#include <nlohmann/json.hpp>

struct ProfileDesc
{
    float StapmLimit = std::numeric_limits<float>::quiet_NaN();
    float PptLimitFast = std::numeric_limits<float>::quiet_NaN();
    float PptLimitSlow = std::numeric_limits<float>::quiet_NaN();

    void LoadFromJson(const nlohmann::json& json);
};

struct Config
{
    std::map<std::string, ProfileDesc> Profiles;
    int RefreshIntervalMs = 3000;

    void LoadFromJson(const nlohmann::json& json);
    void LoadFromFile();
};
