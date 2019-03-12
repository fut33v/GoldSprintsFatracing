// Copyright 2018

#ifndef SUBPRJ_VISMUTINPROTOTYPEMODULE_SRC_MAIN_H_
#define SUBPRJ_VISMUTINPROTOTYPEMODULE_SRC_MAIN_H_

#include <QMainWindow>
#include <QGraphicsScene>
//#include "ui_vismutinprototypemodule.h"


class GoldSprintsFatracing : public QMainWindow {
	Q_OBJECT

public:
    explicit GoldSprintsFatracing(QWidget* parent = nullptr);
    ~GoldSprintsFatracing();

//private slots:

//signals:
    
protected:
    void paintEvent(QPaintEvent *) override;

private:
//    Ui::VismutinPrototypeModuleClass ui;
};

#endif // SUBPRJ_VISMUTINPROTOTYPEMODULE_SRC_MAIN_H_
