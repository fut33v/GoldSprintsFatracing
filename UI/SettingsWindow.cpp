#include "./SettingsWindow.h"


SettingsWindow::SettingsWindow(QWidget* parent) : QMainWindow(parent), mLogger(Fatracing::Logger::Instance()) {
    ui.setupUi(this);

}

SettingsWindow::~SettingsWindow() {

}






