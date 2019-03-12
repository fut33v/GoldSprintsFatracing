// Copyright 2018

#ifndef GOLDSPRINTSFATRACING_H
#define GOLDSPRINTSFATRACING_H

#include <QMainWindow>
#include <QGraphicsScene>

#include "Logger.h"

#include "ui_goldsprintsfatracing.h"


class GoldSprintsFatracing : public QMainWindow {
	Q_OBJECT

public:
    explicit GoldSprintsFatracing(QWidget* parent = nullptr);
    ~GoldSprintsFatracing();

private slots:
    void OnPushButtonRace();
//signals:

private:
    Fatracing::Logger& mLogger;
    Ui_GoldSprintsFatracing ui;
//    Ui::VismutinPrototypeModuleClass;

};

#endif // GOLDSPRINTSFATRACING_H
