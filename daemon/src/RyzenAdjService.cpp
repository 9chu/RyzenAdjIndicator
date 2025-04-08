#include "RyzenAdjService.hpp"

#include <cassert>
#include <stdexcept>
#include <fmt/format.h>
#include <ryzenadj_priv.h>

using namespace std;

inline const char* ToString(ryzen_family family)
{
	switch (family)
	{
	case FAM_RAVEN: return "Raven";
	case FAM_PICASSO: return "Picasso";
	case FAM_RENOIR: return "Renoir";
	case FAM_CEZANNE: return "Cezanne";
	case FAM_DALI: return "Dali";
	case FAM_LUCIENNE: return "Lucienne";
	case FAM_VANGOGH: return "Vangogh";
	case FAM_REMBRANDT: return "Rembrandt";
	case FAM_PHOENIX: return "Phoenix Point";
	case FAM_HAWKPOINT: return "Hawk Point";
	case FAM_STRIXPOINT: return "Strix Point";
	default: return "Unknown";
	}
}

RyzenAdjService& RyzenAdjService::GetInstance()
{
	static RyzenAdjService kInstance;
	return kInstance;
}

RyzenAdjService::RyzenAdjService()
{
}

RyzenAdjService::~RyzenAdjService()
{
	if (m_pRyzenAccess)
		::cleanup_ryzenadj(m_pRyzenAccess);
}

void RyzenAdjService::Init()
{
	assert(!m_pRyzenAccess);

	// 初始化 RyzenAdj
	m_pRyzenAccess = ::init_ryzenadj();
	if (!m_pRyzenAccess)
		throw std::runtime_error("Failed to initialize RyzenAdj");

	// 初始化电源指标表
	// 理论上初始化失败不影响调整功耗的功能，但是为了功能完整性，这里还是强制失败
	auto err = ::init_table(m_pRyzenAccess);
	if (err)
	{
		::cleanup_ryzenadj(m_pRyzenAccess);
		m_pRyzenAccess = nullptr;
		throw std::runtime_error(fmt::format("Failed to initialize power metric table: {}", err));
	}
}

InfoDesc RyzenAdjService::GetInfo()
{
	std::unique_lock<std::mutex> lock(m_stRyzenAccessLock);

	InfoDesc ret = {
		.CpuFamily = ToString(::get_cpu_family(m_pRyzenAccess)),
		.SmuBiosInterfaceVer = ::get_bios_if_ver(m_pRyzenAccess),
	};
	return ret;
}

PowerMetricsTable RyzenAdjService::GetPowerMetricsTable()
{
	std::unique_lock<std::mutex> lock(m_stRyzenAccessLock);

	auto ec = ::refresh_table(m_pRyzenAccess);
	if (ec)
		throw std::runtime_error(fmt::format("Failed to refresh power metric table: {}", ec));

	PowerMetricsTable ret = {
		.StapmLimit = ::get_stapm_limit(m_pRyzenAccess),
		.StapmValue = ::get_stapm_value(m_pRyzenAccess),
		.PptLimitFast = ::get_fast_limit(m_pRyzenAccess),
		.PptValueFast = ::get_fast_value(m_pRyzenAccess),
		.PptLimitSlow = ::get_slow_limit(m_pRyzenAccess),
		.PptValueSlow = ::get_slow_value(m_pRyzenAccess),
		.StapmTimeConst = ::get_stapm_time(m_pRyzenAccess),
		.SlowPptTimeConst = ::get_slow_time(m_pRyzenAccess),
		.PptLimitApu = ::get_apu_slow_limit(m_pRyzenAccess),
		.PptValueApu = ::get_apu_slow_value(m_pRyzenAccess),
		.TdcLimitVdd = ::get_vrm_current(m_pRyzenAccess),
		.TdcValueVdd = ::get_vrm_current_value(m_pRyzenAccess),
		.TdcLimitSoc = ::get_vrmsoc_current(m_pRyzenAccess),
		.TdcValueSoc = ::get_vrmsoc_current_value(m_pRyzenAccess),
		.EdcLimitVdd = ::get_vrmmax_current(m_pRyzenAccess),
		.EdcValueVdd = ::get_vrmmax_current_value(m_pRyzenAccess),
		.EdcLimitSoc = ::get_vrmsocmax_current(m_pRyzenAccess),
		.EdcValueSoc = ::get_vrmsocmax_current_value(m_pRyzenAccess),
		.ThmLimitCore = ::get_tctl_temp(m_pRyzenAccess),
		.ThmValueCore = ::get_tctl_temp_value(m_pRyzenAccess),
		.SttLimitApu = ::get_apu_skin_temp_limit(m_pRyzenAccess),
		.SttValueApu = ::get_apu_skin_temp_value(m_pRyzenAccess),
		.SttLimitDGpu = ::get_dgpu_skin_temp_limit(m_pRyzenAccess),
		.SttValueDGpu = ::get_dgpu_skin_temp_value(m_pRyzenAccess),
		.CclkBoostSetPoint = ::get_cclk_setpoint(m_pRyzenAccess),
		.CclkBusyValue = ::get_cclk_busy_value(m_pRyzenAccess),
	};
	return ret;
}

int RyzenAdjService::SetStapmLimit(float value)
{
	std::unique_lock<std::mutex> lock(m_stRyzenAccessLock);
	return ::set_stapm_limit(m_pRyzenAccess, static_cast<uint32_t>(value * 1000));
}

int RyzenAdjService::SetFastLimit(float value)
{
	std::unique_lock<std::mutex> lock(m_stRyzenAccessLock);
	return ::set_fast_limit(m_pRyzenAccess, static_cast<uint32_t>(value * 1000));
}

int RyzenAdjService::SetSlowLimit(float value)
{
	std::unique_lock<std::mutex> lock(m_stRyzenAccessLock);
	return ::set_slow_limit(m_pRyzenAccess, static_cast<uint32_t>(value * 1000));
}
