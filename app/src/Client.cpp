#include "Client.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;

const char* kSocketPath = "/tmp/ryzen-adj-daemon.sock";

Client::Client()
{
    // 定位 Daemon 进程
    auto currentBinaryPath = filesystem::canonical("/proc/self/exe");
    auto daemonBinaryPath = currentBinaryPath.parent_path() / "RyzenAdjDaemon";
    daemonBinaryPath = filesystem::absolute(daemonBinaryPath);
    if (!filesystem::exists(daemonBinaryPath))
        throw runtime_error("Daemon binary missing");

    // 提权删除旧的 Socket 文件
    if (filesystem::exists(kSocketPath))
    {
        fprintf(stdout, "Daemon socket file exists, removing...\n");

        auto pid = ::fork();
        if (pid == 0)
        {
            const char* argv[] = { "pkexec", "/bin/rm", kSocketPath, nullptr };
            ::execvp("pkexec", const_cast<char* const*>(argv));
            _exit(1);  // execvp 失败时退出子进程
        }

        int status = 0;
        ::waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) != 0)
            throw runtime_error("Failed to remove old socket file");
    }

    // 启动 Daemon 进程
    auto pid = ::fork();
    if (pid == 0)
    {
        const char* argv[] = { "pkexec", daemonBinaryPath.c_str(), nullptr };
        ::execvp("pkexec", const_cast<char* const*>(argv));
        _exit(1);  // execvp 失败时退出子进程
    }

    // 等待 Daemon 进程启动
    while (true)
    {
        // 检查 Daemon 进程是否存在
        int status = 0;
        if (0 != ::waitpid(pid, &status, WNOHANG))
        {
            fprintf(stderr, "Daemon process exited unexpectedly, code: %d\n", status);
            assert(WIFEXITED(status));
            if (WEXITSTATUS(status) == 126)
                throw runtime_error("Failed to grant permission");
            throw runtime_error("Failed to start daemon process");
        }

        // 检查 Socket 是否存在
        if (filesystem::exists(kSocketPath))
            break;

        ::sleep(1);
    }
    fprintf(stdout, "Daemon startup, pid: %d\n", pid);
    m_iChildProcPid = pid;

    // 连接到 Daemon 进程的 Unix Socket
    m_iSocket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_iSocket < 0)
        throw runtime_error("Failed to create socket");

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
    if (::connect(m_iSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        ::close(m_iSocket);
        throw runtime_error("Failed to connect to socket");
    }
}

Client::~Client()
{
    if (m_iSocket != -1)
    {
        try
        {
            SendRequest("exit");
        }
        catch (const exception& e)
        {
        }

        ::close(m_iSocket);
    }

    if (m_iChildProcPid != 0)
    {
        int status = 0;
        ::waitpid(m_iChildProcPid, &status, 0);
    }
}

InfoDesc Client::GetInfo()
{
    auto data = SendRequest("get_info");
    InfoDesc info;
    info.CpuFamily = data["cpu_family"].get<string>();
    info.SmuBiosInterfaceVer = data["smu_bios_if_ver"].get<int>();
    return info;
}

inline float GetFloatValue(const nlohmann::json& data, const char* key)
{
    if (!data.contains(key))
        return numeric_limits<float>::quiet_NaN();
    auto& v = data[key];
    if (v.is_number_float())
        return v.get<float>();
    return numeric_limits<float>::quiet_NaN();
}

PowerMetricsTable Client::GetPowerMetricsTable()
{
    auto data = SendRequest("get_metrics_table");
    PowerMetricsTable table;
    table.StapmLimit = GetFloatValue(data, "stapm_limit");
    table.StapmValue = GetFloatValue(data, "stapm_value");
    table.PptLimitFast = GetFloatValue(data, "ppt_limit_fast");
    table.PptValueFast = GetFloatValue(data, "ppt_value_fast");
    table.PptLimitSlow = GetFloatValue(data, "ppt_limit_slow");
    table.PptValueSlow = GetFloatValue(data, "ppt_value_slow");
    table.StapmTimeConst = GetFloatValue(data, "stapm_time_const");
    table.SlowPptTimeConst = GetFloatValue(data, "slow_ppt_time_const");
    table.PptLimitApu = GetFloatValue(data, "ppt_limit_apu");
    table.PptValueApu = GetFloatValue(data, "ppt_value_apu");
    table.TdcLimitVdd = GetFloatValue(data, "tdc_limit_vdd");
    table.TdcValueVdd = GetFloatValue(data, "tdc_value_vdd");
    table.TdcLimitSoc = GetFloatValue(data, "tdc_limit_soc");
    table.TdcValueSoc = GetFloatValue(data, "tdc_value_soc");
    table.EdcLimitVdd = GetFloatValue(data, "edc_limit_vdd");
    table.EdcValueVdd = GetFloatValue(data, "edc_value_vdd");
    table.EdcLimitSoc = GetFloatValue(data, "edc_limit_soc");
    table.EdcValueSoc = GetFloatValue(data, "edc_value_soc");
    table.ThmLimitCore = GetFloatValue(data, "thm_limit_core");
    table.ThmValueCore = GetFloatValue(data, "thm_value_core");
    table.SttLimitApu = GetFloatValue(data, "stt_limit_apu");
    table.SttValueApu = GetFloatValue(data, "stt_value_apu");
    table.SttLimitDGpu = GetFloatValue(data, "stt_limit_dgpu");
    table.SttValueDGpu = GetFloatValue(data, "stt_value_dgpu");
    table.CclkBoostSetPoint = GetFloatValue(data, "cclk_boost_setpoint");
    table.CclkBusyValue = GetFloatValue(data, "cclk_busy_value");
    return table;
}

void Client::SetStapmLimit(float value)
{
    SendRequest("set_stapm_limit", &value);
}

void Client::SetFastLimit(float value)
{
    SendRequest("set_fast_limit", &value);
}

void Client::SetSlowLimit(float value)
{
    SendRequest("set_slow_limit", &value);
}

nlohmann::json Client::SendRequest(const char* req, float* value)
{
    auto payload = nlohmann::json {
        {"req", req},
    };
    if (value)
        payload["value"] = *value;
    auto payloadStr = payload.dump();
    uint32_t payloadLen = payloadStr.length();
    ::send(m_iSocket, &payloadLen, sizeof(payloadLen), 0);
    auto sendLen = ::send(m_iSocket, payloadStr.c_str(), payloadLen, 0);
    if (sendLen != payloadLen)
        throw runtime_error("Failed to send request");

    uint32_t respLen = 0;
    auto recvLen = ::recv(m_iSocket, &respLen, sizeof(respLen), 0);
    if (recvLen != sizeof(respLen))
        throw runtime_error("Failed to receive response length");
    auto respStr = string(respLen, '\0');
    recvLen = ::recv(m_iSocket, &respStr[0], respLen, 0);
    if (recvLen != respLen)
        throw runtime_error("Failed to receive response");
    auto resp = nlohmann::json::parse(respStr);
    if (resp["status"].get<int>() != 0)
        throw runtime_error(fmt::format("Daemon error: {}", resp["msg"].get<string>()));
    return resp["data"];
}
