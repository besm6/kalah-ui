#include <gtkmm.h>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
    auto app = Gtk::Application::create("org.besm6.kalah");
    return app->make_window_and_run<MainWindow>(argc, argv);
}
