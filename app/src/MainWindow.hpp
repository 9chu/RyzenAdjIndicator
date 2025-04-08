#pragma once
#include <gtkmm.h>
#include <libayatana-appindicator/app-indicator.h>
#include "Client.hpp"
#include "Config.hpp"

class MainWindow :
    public Gtk::Window
{
public:
    MainWindow(Gtk::Application* app);
    ~MainWindow() override;

protected:
    bool OnRefreshIndicator();
    void OnSwitchClicked(const std::string& profileName);
    void OnExitClicked();

private:
    Gtk::Application* m_pApp = nullptr;
    Config m_stConfig;
    std::unique_ptr<Client> m_pClient;

    Gtk::Menu m_stMainMenu;
    Gtk::MenuItem m_stStapmWattMenuItem;
    Gtk::MenuItem m_stFastWattMenuItem;
    Gtk::MenuItem m_stSlowWattMenuItem;
    Gtk::MenuItem m_stThmCoreMenuItem;
    Gtk::SeparatorMenuItem m_stSeperator1;
    std::vector<Gtk::MenuItem> m_stLimitSwitchMenuItems;
    Gtk::SeparatorMenuItem m_stSeperator2;
    Gtk::MenuItem m_stExitMenuItem;

    AppIndicator* m_pAppIndicator = nullptr;
};
