#include "MainWindow.hpp"

#include <filesystem>
#include <fmt/format.h>

using namespace std;

#define NaN numeric_limits<float>::quiet_NaN()

namespace
{
    std::string MakeWattLabel(const char* label, float current, float limit)
    {
        return fmt::format("{0}: {1:.1f} / {2:.1f} W", label, current, limit);
    }

    std::string MakeTempLabel(const char* label, float current, float limit)
    {
        return fmt::format("{0}: {1:.1f} / {2:.1f} ℃", label, current, limit);
    }
}

MainWindow::MainWindow(Gtk::Application* app)
    : m_pApp(app)
{
    // 隐藏主窗口
    set_visible(false);

    // 加载配置
    try
    {
        m_stConfig.LoadFromFile();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Failed to load config file: %s\n", ex.what());
        m_stConfig = {};
    }

    // 初始化 RyzenAdj 后端
    try
    {
        m_pClient = make_unique<Client>();
    }
    catch (const std::exception& ex)
    {
        Gtk::MessageDialog messageBox(fmt::format("Unable to initialize RyzenAdj.\n\n{}", ex.what()), true,
            Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
        messageBox.set_title("Error");
        messageBox.set_modal();
        messageBox.set_position(Gtk::WindowPosition::WIN_POS_CENTER);
        messageBox.run();
        exit(1);
    }

    // 配置菜单
    m_stStapmWattMenuItem = Gtk::MenuItem(MakeWattLabel("STAPM", NaN, NaN)); m_stStapmWattMenuItem.show();
    m_stFastWattMenuItem = Gtk::MenuItem(MakeWattLabel("PPT FAST", NaN, NaN)); m_stFastWattMenuItem.show();
    m_stSlowWattMenuItem = Gtk::MenuItem(MakeWattLabel("PPT SLOW", NaN, NaN)); m_stSlowWattMenuItem.show();
    m_stThmCoreMenuItem = Gtk::MenuItem(MakeTempLabel("THM CORE", NaN, NaN)); m_stThmCoreMenuItem.show();
    m_stSeperator1.show();
    for (auto& item : m_stConfig.Profiles)
    {
        auto& profileName = item.first;
        auto& profileDesc = item.second;
        auto menuItem = Gtk::MenuItem(profileName); menuItem.show();
        m_stLimitSwitchMenuItems.emplace_back(std::move(menuItem));
    }
    m_stSeperator2.show();
    m_stExitMenuItem = Gtk::MenuItem("Exit"); m_stExitMenuItem.show();

    m_stMainMenu.append(m_stStapmWattMenuItem);
    m_stMainMenu.append(m_stFastWattMenuItem);
    m_stMainMenu.append(m_stSlowWattMenuItem);
    m_stMainMenu.append(m_stThmCoreMenuItem);
    m_stMainMenu.append(m_stSeperator1);
    for (auto& item : m_stLimitSwitchMenuItems)
        m_stMainMenu.append(item);
    m_stMainMenu.append(m_stSeperator2);
    m_stMainMenu.append(m_stExitMenuItem);

    // 初始化指示器
    m_pAppIndicator = ::app_indicator_new("ryzen-adj-indicator", "indicator-messages",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    g_assert(G_IS_OBJECT(m_pAppIndicator));

    auto currentBinaryPath = filesystem::canonical("/proc/self/exe");
    auto assetDir = currentBinaryPath.parent_path() / "assets";
    if (filesystem::exists(assetDir / "icon.png"))
    {
        ::app_indicator_set_icon_theme_path(m_pAppIndicator, assetDir.c_str());
        ::app_indicator_set_icon_full(m_pAppIndicator, "icon", "icon");
    }

    ::app_indicator_set_status(m_pAppIndicator, APP_INDICATOR_STATUS_ACTIVE);
    ::app_indicator_set_title(m_pAppIndicator, "AMD Ryzen CPU TDP Controller");
    ::app_indicator_set_menu(m_pAppIndicator, m_stMainMenu.gobj());

    // 配置逻辑
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::OnRefreshIndicator), m_stConfig.RefreshIntervalMs);
    for (auto& item : m_stLimitSwitchMenuItems)
    {
        item.signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &MainWindow::OnSwitchClicked),
            item.get_label()));
    }
    m_stExitMenuItem.signal_activate().connect(sigc::mem_fun(*this, &MainWindow::OnExitClicked));
}

MainWindow::~MainWindow()
{
    if (m_pAppIndicator)
    {
        ::g_object_unref(m_pAppIndicator);
        m_pAppIndicator = nullptr;
    }
}

bool MainWindow::OnRefreshIndicator()
{
    float stapmValue = NaN, stapmLimit = NaN, slowValue = NaN, slowLimit = NaN,
        fastValue = NaN, fastLimit = NaN, thmValue = NaN, thmLimit = NaN;

    try
    {
        auto metrics = m_pClient->GetPowerMetricsTable();
        stapmValue = metrics.StapmValue;
        stapmLimit = metrics.StapmLimit;
        slowValue = metrics.PptValueSlow;
        slowLimit = metrics.PptLimitSlow;
        fastValue = metrics.PptValueFast;
        fastLimit = metrics.PptLimitFast;
        thmValue = metrics.ThmValueCore;
        thmLimit = metrics.ThmLimitCore;
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "Cannot get power metrics table: %s\n", ex.what());
    }

    m_stStapmWattMenuItem.set_label(MakeWattLabel("STAPM", stapmValue, stapmLimit));
    m_stFastWattMenuItem.set_label(MakeWattLabel("PPT FAST", fastValue, fastLimit));
    m_stSlowWattMenuItem.set_label(MakeWattLabel("PPT SLOW", slowValue, slowLimit));
    m_stThmCoreMenuItem.set_label(MakeTempLabel("THM CORE", thmValue, thmLimit));
    return true;
}

void MainWindow::OnSwitchClicked(const std::string& profileName)
{
    fprintf(stdout, "OnSwitchClicked, profile=%s\n", profileName.c_str());
    auto it = m_stConfig.Profiles.find(profileName);
    if (it == m_stConfig.Profiles.end())
        return;

    if (!std::isnan(it->second.StapmLimit))
    {
        fprintf(stdout, "Set STAPM Limit: %f\n", it->second.StapmLimit);
        try
        {
            m_pClient->SetStapmLimit(it->second.StapmLimit);
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "Failed to set STAPM limit: %s\n", ex.what());

            Gtk::MessageDialog messageBox(fmt::format("SetStapmLimit fail.\n\n{}", ex.what()), true,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            messageBox.set_title("Error");
            messageBox.set_modal();
            messageBox.set_position(Gtk::WindowPosition::WIN_POS_CENTER);
            messageBox.run();
        }
    }
    if (!std::isnan(it->second.PptLimitFast))
    {
        fprintf(stdout, "Set PPT Limit Fast: %f\n", it->second.PptLimitFast);
        try
        {
            m_pClient->SetFastLimit(it->second.PptLimitFast);
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "Failed to set PPT Limit Fast: %s\n", ex.what());

            Gtk::MessageDialog messageBox(fmt::format("PptLimitFast fail.\n\n{}", ex.what()), true,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            messageBox.set_title("Error");
            messageBox.set_modal();
            messageBox.set_position(Gtk::WindowPosition::WIN_POS_CENTER);
            messageBox.run();
        }
    }
    if (!std::isnan(it->second.PptLimitSlow))
    {
        fprintf(stdout, "Set PPT Limit Slow: %f\n", it->second.PptLimitSlow);
        try
        {
            m_pClient->SetSlowLimit(it->second.PptLimitSlow);
        }
        catch (const std::exception& ex)
        {
            fprintf(stderr, "Failed to set PPT Limit Slow: %s\n", ex.what());

            Gtk::MessageDialog messageBox(fmt::format("PptLimitSlow fail.\n\n{}", ex.what()), true,
                Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
            messageBox.set_title("Error");
            messageBox.set_modal();
            messageBox.set_position(Gtk::WindowPosition::WIN_POS_CENTER);
            messageBox.run();
        }
    }
}

void MainWindow::OnExitClicked()
{
    fprintf(stdout, "OnExitClicked\n");
    ::app_indicator_set_status(m_pAppIndicator, APP_INDICATOR_STATUS_PASSIVE);
    m_pApp->quit();
}
