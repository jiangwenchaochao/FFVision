#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FFVision.h"

class FFVision : public QMainWindow
{
    Q_OBJECT

public:
    FFVision(QWidget *parent = nullptr);
    ~FFVision();

private:
    Ui::FFVisionClass ui;
};

