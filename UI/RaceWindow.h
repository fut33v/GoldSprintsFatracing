#ifndef RACE_WINDOW_H_
#define RACE_WINDOW_H_

#include <QMainWindow>
#include <QGraphicsScene>

#include "Logger.h"

#include "ui_racewindow.h"


class RaceWindow : public QMainWindow {
	Q_OBJECT

public:
    explicit RaceWindow(QWidget* parent = nullptr);
    ~RaceWindow();
private:
    Fatracing::Logger& mLogger;
    Ui_RaceWindow ui;

private slots:
    void OnPushButtonStart();

};

#endif // RACE_WINDOW_H_
