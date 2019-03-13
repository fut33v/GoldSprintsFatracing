#ifndef RACE_WINDOW_H_
#define RACE_WINDOW_H_

#include <QMainWindow>
#include <QGraphicsScene>

#include <memory>

#include "Logger.h"

#include "Core/Race.h"

#include "ui_racewindow.h"


Q_DECLARE_METATYPE(Fatracing::RaceStruct)

class RaceWindow : public QMainWindow {
	Q_OBJECT

public:
    explicit RaceWindow(QWidget* parent = nullptr);
    ~RaceWindow();
private:
    Fatracing::Logger& mLogger;
    std::shared_ptr<Fatracing::Race> mRace;

    Ui_RaceWindow ui;

    void RaceCallback(Fatracing::RaceStruct);

signals:
    void RaceSignal(Fatracing::RaceStruct);

private slots:
    void OnPushButtonStart();
    void RaceSlot(Fatracing::RaceStruct);
};

#endif // RACE_WINDOW_H_
