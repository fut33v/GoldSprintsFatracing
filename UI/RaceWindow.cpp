#include "./RaceWindow.h"


RaceWindow::RaceWindow(QWidget* parent) : QMainWindow(parent), mLogger(Fatracing::Logger::Instance()) {
    ui.setupUi(this);

    connect(ui.pushButtonStart, &QPushButton::clicked, this, &RaceWindow::OnPushButtonStart);

    // create new race object
    // give settings to it
    //


}

RaceWindow::~RaceWindow() {

}

void RaceWindow::OnPushButtonStart() {

    // mRace.SetUpCallback(this.someCallbackToGetData);
    // mRace.Start();

}






