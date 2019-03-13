#include "./RaceWindow.h"
#include "Core/Settings.h"
#include <functional>

RaceWindow::RaceWindow(QWidget* parent) : QMainWindow(parent), mLogger(Fatracing::Logger::Instance()) {
    ui.setupUi(this);

    connect(ui.pushButtonStart, &QPushButton::clicked, this, &RaceWindow::OnPushButtonStart);
    connect(ui.pushButtonClear, &QPushButton::clicked, this, &RaceWindow::OnPushButtonClear);
    connect(this, &RaceWindow::RaceSignal, this, &RaceWindow::RaceSlot, Qt::QueuedConnection);

    auto s = Fatracing::SettingsSingleton::Instance().GetSettings();
    mRace = std::make_shared<Fatracing::Race>(s, std::bind(&RaceWindow::RaceCallback, this, std::placeholders::_1));

	qRegisterMetaType<Fatracing::RaceStruct>();
}

RaceWindow::~RaceWindow() {

}

void RaceWindow::RaceCallback(Fatracing::RaceStruct aRaceStruct) {
    emit RaceSignal(aRaceStruct);
}

void RaceWindow::OnPushButtonStart() {
    ui.pushButtonStart->setEnabled(false);
    ui.pushButtonClear->setEnabled(false);
    ui.lineEditBlue->setEnabled(false);
    ui.lineEditRed->setEnabled(false);

    mRace->Start();
}

void RaceWindow::RaceSlot(Fatracing::RaceStruct aRaceStruct) {
    QString seconds = "0:";
    ui.labelSeconds->setText(seconds + QString::number(aRaceStruct.Seconds));

    ui.labelBlueScore->setText(QString::number(aRaceStruct.BlueScore));
    ui.labelRedScore->setText(QString::number(aRaceStruct.RedScore));

    ui.labelBlueRPM->setText(QString::number(aRaceStruct.BlueRPM));
    ui.labelRedRPM->setText(QString::number(aRaceStruct.RedRPM));

    if (aRaceStruct.Finish) {
        ui.lineEditBlue->setText(QString::number(aRaceStruct.BlueScore));
        ui.lineEditRed->setText(QString::number(aRaceStruct.RedScore));
        ui.pushButtonStart->setEnabled(true);
        ui.pushButtonClear->setEnabled(true);
        ui.lineEditBlue->setEnabled(true);
        ui.lineEditRed->setEnabled(true);
    }

    if (aRaceStruct.BlueScore != 0 && aRaceStruct.RedScore != 0) {
        switch (aRaceStruct.Leader) {
        case Fatracing::RacersEnum::BLUE: {
            ui.labelLeader->setText("BLUE");
            break;
        }
        case Fatracing::RacersEnum::RED: {
            ui.labelLeader->setText("RED");
            break;
        }
        }

        ui.labelDiff->setText(QString::number(aRaceStruct.Diff));
    }
}






