#pragma once
#include <nlohmann/json.hpp>

struct InfoDesc
{
    std::string CpuFamily;
    int SmuBiosInterfaceVer = 0;
};

struct PowerMetricsTable
{
    float StapmLimit = 0;
    float StapmValue = 0;
    float PptLimitFast = 0;
    float PptValueFast = 0;
    float PptLimitSlow = 0;
    float PptValueSlow = 0;
    float StapmTimeConst = 0;
    float SlowPptTimeConst = 0;
    float PptLimitApu = 0;
    float PptValueApu = 0;
    float TdcLimitVdd = 0;
    float TdcValueVdd = 0;
    float TdcLimitSoc = 0;
    float TdcValueSoc = 0;
    float EdcLimitVdd = 0;
    float EdcValueVdd = 0;
    float EdcLimitSoc = 0;
    float EdcValueSoc = 0;
    float ThmLimitCore = 0;
    float ThmValueCore = 0;
    float SttLimitApu = 0;
    float SttValueApu = 0;
    float SttLimitDGpu = 0;
    float SttValueDGpu = 0;
    float CclkBoostSetPoint = 0;
    float CclkBusyValue = 0;
};

class Client
{
public:
    Client();
    ~Client();

public:
    InfoDesc GetInfo();
    PowerMetricsTable GetPowerMetricsTable();
    void SetStapmLimit(float value);
    void SetFastLimit(float value);
    void SetSlowLimit(float value);

private:
    nlohmann::json SendRequest(const char* req, float* value = nullptr);

private:
    int m_iSocket = -1;
    int m_iChildProcPid = 0;
};
