#pragma once
#include <mutex>
#include <ryzenadj.h>

struct InfoDesc
{
    const char* CpuFamily = nullptr;
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

class RyzenAdjService
{
public:
    static RyzenAdjService& GetInstance();

protected:
    RyzenAdjService();

public:
    RyzenAdjService(const RyzenAdjService&) = delete;
    RyzenAdjService& operator=(const RyzenAdjService&) = delete;
    ~RyzenAdjService();

public:
    void Init();

    InfoDesc GetInfo();
    PowerMetricsTable GetPowerMetricsTable();

    int SetStapmLimit(float value);  // 5.0 -> 5 Watt
    int SetFastLimit(float value);
    int SetSlowLimit(float value);

private:
    ryzen_access m_pRyzenAccess = nullptr;
    std::mutex m_stRyzenAccessLock;
};
