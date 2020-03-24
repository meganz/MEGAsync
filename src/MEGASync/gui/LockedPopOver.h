#pragma once

#include <QWidget>
#include "ui_LockedPopOver.h"

namespace Ui {
class LockedPopOver;
}

class LockedPopOver : public QWidget
{
    Q_OBJECT

public:
    explicit LockedPopOver(QWidget *parent = 0);
    LockedPopOver();

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent* event);

private:
    Ui::LockedPopOver *ui;
};


