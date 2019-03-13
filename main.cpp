#include <QtWidgets/QApplication>
//#include "UI/GoldSprintsFatracing.h"
#include "UI/RaceWindow.h"
#include "Core/Settings.h"


int main(int argc, char *argv[]) {
    Fatracing::SettingsSingleton::Instance().LoadSettings();
	QApplication a(argc, argv);
    //GoldSprintsFatracing w;
    RaceWindow w;
    w.show();
	return a.exec();
}
