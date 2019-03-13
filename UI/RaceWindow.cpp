#include "./RaceWindow.h"
#include "Core/Settings.h"
#include <functional>

RaceWindow::RaceWindow(QWidget* parent) : QMainWindow(parent), mLogger(Fatracing::Logger::Instance()) {
    ui.setupUi(this);

    connect(ui.pushButtonStart, &QPushButton::clicked, this, &RaceWindow::OnPushButtonStart);
    connect(this, &RaceWindow::RaceSignal, this, &RaceWindow::RaceSlot, Qt::QueuedConnection);

    // create new race object
    // give settings to it
    //
    auto s = Fatracing::SettingsSingleton::Instance().GetSettings();
    mRace = std::make_shared<Fatracing::Race>(s, std::bind(&RaceWindow::RaceCallback, this, std::placeholders::_1));

	qRegisterMetaType<Fatracing::RaceStruct>();
}

RaceWindow::~RaceWindow() {

}

void RaceWindow::RaceCallback(Fatracing::RaceStruct aRaceStruct) {
    // invoke signal
    emit RaceSignal(aRaceStruct);
}

void RaceWindow::OnPushButtonStart() {
    mRace->Start();
    // mRace.Start();

}

void RaceWindow::RaceSlot(Fatracing::RaceStruct aRaceStruct) {
    ui.labelBlueScore->setText(QString::number(aRaceStruct.BlueScore));
    ui.labelRedScore->setText(QString::number(aRaceStruct.RedScore));
}






