#include "GoldSprintsFatracing.h"
#include "RaceWindow.h"


GoldSprintsFatracing::GoldSprintsFatracing(QWidget* parent) : QMainWindow(parent),
                                                                    mLogger(Fatracing::Logger::Instance()) {
    ui.setupUi(this);

    connect(ui.pushButtonRace, &QPushButton::clicked, this, &GoldSprintsFatracing::OnPushButtonRace);
//	setlocale(LC_ALL, "ru_RU.UTF-8");

//    mLogger.AddCallback("ui", std::bind(&GoldSprintsFatracing::LoggerCallbackTable, this, std::placeholders::_1));
}

GoldSprintsFatracing::~GoldSprintsFatracing() {

}

void GoldSprintsFatracing::OnPushButtonRace() {
    RaceWindow* raceWindow = new RaceWindow(this);
    raceWindow->show();
    hide();
}






