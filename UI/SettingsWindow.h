// Copyright 2018

#ifndef SETTINGS_WINDOW_H_
#define SETTINGS_WINDOW_H_

#include <QMainWindow>
#include <QGraphicsScene>

#include "Logger.h"

#include "ui_settingswindow.h"


class SettingsWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget* parent = nullptr);
    ~SettingsWindow();
private:
    Fatracing::Logger& mLogger;
    Ui_SettingsWindow ui;
};

#endif // SETTINGS_WINDOW_H_
