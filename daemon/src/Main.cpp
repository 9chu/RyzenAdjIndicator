#include <fmt/format.h>
#include "Server.hpp"
#include "RyzenAdjService.hpp"

const char* kSocketPath = "/tmp/ryzen-adj-daemon.sock";

enum
{
    STATUS_OK = 0,
    STATUS_UNKNOWN_REQ = -1,
    STATUS_SERVICE_UNAVAILABLE = -2,
    STATUS_BAD_ARGUMENTS = -3,
    STATUS_NOT_SUPPORTED = -4,
};

static Server* kInstance = nullptr;

static void RequestHandler(SessionPtr session, const nlohmann::json& payload)
{
    if (!payload.is_object())
    {
        session->Abort();
        return;
    }

    auto req = payload.value("req", "");
    auto resp = nlohmann::json {
        {"status", STATUS_OK},
        {"msg", "ok"},
        {"data", nlohmann::json::object()}
    };
    auto& respData = resp["data"];

    bool closeConnection = false;
    if (req == "ping")
    {
    }
    if (req == "exit")
    {
        fprintf(stdout, "Exit received\n");
        if (kInstance)
            kInstance->Stop();
        closeConnection = true;
    }
    else if (req == "get_info")
    {
        auto info = RyzenAdjService::GetInstance().GetInfo();
        respData["cpu_family"] = info.CpuFamily;
        respData["smu_bios_if_ver"] = info.SmuBiosInterfaceVer;
        respData["ryzen_adj_ver"] = fmt::format("{}.{}.{}", RYZENADJ_REVISION_VER, RYZENADJ_MAJOR_VER,
            RYZENADJ_MINIOR_VER);
    }
    else if (req == "get_metrics_table")
    {
        auto table = RyzenAdjService::GetInstance().GetPowerMetricsTable();
        respData["stapm_limit"] = table.StapmLimit;
        respData["stapm_value"] = table.StapmValue;
        respData["ppt_limit_fast"] = table.PptLimitFast;
        respData["ppt_value_fast"] = table.PptValueFast;
        respData["ppt_limit_slow"] = table.PptLimitSlow;
        respData["ppt_value_slow"] = table.PptValueSlow;
        respData["stapm_time_const"] = table.StapmTimeConst;
        respData["slow_ppt_time_const"] = table.SlowPptTimeConst;
        respData["ppt_limit_apu"] = table.PptLimitApu;
        respData["ppt_value_apu"] = table.PptValueApu;
        respData["tdc_limit_vdd"] = table.TdcLimitVdd;
        respData["tdc_value_vdd"] = table.TdcValueVdd;
        respData["tdc_limit_soc"] = table.TdcLimitSoc;
        respData["tdc_value_soc"] = table.TdcValueSoc;
        respData["edc_limit_vdd"] = table.EdcLimitVdd;
        respData["edc_value_vdd"] = table.EdcValueVdd;
        respData["edc_limit_soc"] = table.EdcLimitSoc;
        respData["edc_value_soc"] = table.EdcValueSoc;
        respData["thm_limit_core"] = table.ThmLimitCore;
        respData["thm_value_core"] = table.ThmValueCore;
        respData["stt_limit_apu"] = table.SttLimitApu;
        respData["stt_value_apu"] = table.SttValueApu;
        respData["stt_limit_dgpu"] = table.SttLimitDGpu;
        respData["stt_value_dgpu"] = table.SttValueDGpu;
        respData["cclk_boost_setpoint"] = table.CclkBoostSetPoint;
        respData["cclk_busy_value"] = table.CclkBusyValue;
    }
    else if (req == "set_stapm_limit")
    {
        if (!payload.contains("value") || !payload["value"].is_number_float())
        {
            resp["status"] = STATUS_BAD_ARGUMENTS;
            resp["msg"] = "Bad arguments";
        }
        else
        {
            auto value = static_cast<float>(payload["value"].get<double>());
            if (value <= 0.0f || std::isnan(value))
            {
                resp["status"] = STATUS_BAD_ARGUMENTS;
                resp["msg"] = "Bad arguments";
            }
            else
            {
                auto err = RyzenAdjService::GetInstance().SetStapmLimit(value);
                if (err == ADJ_ERR_FAM_UNSUPPORTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Not supported on this family";
                }
                else if (err == ADJ_ERR_SMU_UNSUPPORTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Not supported on this SMU";
                }
                else if (err == ADJ_ERR_SMU_REJECTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Rejected by SMU";
                }
                else if (err)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = fmt::format("Unknown error {}", err);
                }
            }
        }
    }
    else if (req == "set_slow_limit")
    {
        if (!payload.contains("value") || !payload["value"].is_number_float())
        {
            resp["status"] = STATUS_BAD_ARGUMENTS;
            resp["msg"] = "Bad arguments";
        }
        else
        {
            auto value = static_cast<float>(payload["value"].get<double>());
            if (value <= 0.0f || std::isnan(value))
            {
                resp["status"] = STATUS_BAD_ARGUMENTS;
                resp["msg"] = "Bad arguments";
            }
            else
            {
                auto err = RyzenAdjService::GetInstance().SetSlowLimit(value);
                if (err == ADJ_ERR_FAM_UNSUPPORTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Not supported on this family";
                }
                else if (err == ADJ_ERR_SMU_UNSUPPORTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Not supported on this SMU";
                }
                else if (err == ADJ_ERR_SMU_REJECTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Rejected by SMU";
                }
                else if (err)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = fmt::format("Unknown error {}", err);
                }
            }
        }
    }
    else if (req == "set_fast_limit")
    {
        if (!payload.contains("value") || !payload["value"].is_number_float())
        {
            resp["status"] = STATUS_BAD_ARGUMENTS;
            resp["msg"] = "Bad arguments";
        }
        else
        {
            auto value = static_cast<float>(payload["value"].get<double>());
            if (value <= 0.0f || std::isnan(value))
            {
                resp["status"] = STATUS_BAD_ARGUMENTS;
                resp["msg"] = "Bad arguments";
            }
            else
            {
                auto err = RyzenAdjService::GetInstance().SetFastLimit(value);
                if (err == ADJ_ERR_FAM_UNSUPPORTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Not supported on this family";
                }
                else if (err == ADJ_ERR_SMU_UNSUPPORTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Not supported on this SMU";
                }
                else if (err == ADJ_ERR_SMU_REJECTED)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = "Rejected by SMU";
                }
                else if (err)
                {
                    resp["status"] = STATUS_NOT_SUPPORTED;
                    resp["msg"] = fmt::format("Unknown error {}", err);
                }
            }
        }
    }
    else
    {
        resp["status"] = STATUS_UNKNOWN_REQ;
        resp["msg"] = fmt::format("Unknown request: {}", req);
        respData.clear();
    }

    session->Send(resp);
    if (closeConnection)
        session->Abort();
}

int main()
{
    // 初始化 RyzenAdj
    try
    {
        RyzenAdjService::GetInstance().Init();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Fatal Error: %s\n", e.what());
        return 1;
    }

    try
    {
        // 启动服务
        Server server(kSocketPath);
        kInstance = &server;
        server.SetHandler(RequestHandler);
        server.Run();
        kInstance = nullptr;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Fatal Error: %s\n", e.what());
        return 2;
    }
    return 0;
}
