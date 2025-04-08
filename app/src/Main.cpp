#include <gtkmm/application.h>
#include "MainWindow.hpp"

static void OnActivateAddMainWindow(Gtk::Application* app, Gtk::Window* window)
{
    app->add_window(*window);
}

int main(int argc, char** argv)
{
    auto app = Gtk::Application::create(argc, argv, "io.github._9chu.ryzen_adj_indicator");
    MainWindow mainWindow(app.get());

    // Gtk always shows our main window, we have to do it manually
    app->signal_activate().connect(sigc::bind(sigc::ptr_fun(&OnActivateAddMainWindow), app.get(), &mainWindow));
    return app->run();
}
