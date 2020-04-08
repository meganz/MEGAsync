#pragma once

#include <QDialog>
#include "ui_LockedPopOver.h"

namespace Ui {
class LockedPopOver;
}

class LockedPopOver : public QDialog
{
    Q_OBJECT

public:
    explicit LockedPopOver(QDialog *parent = 0);
    LockedPopOver();

protected:
    void changeEvent(QEvent *event);
    void showEvent(QShowEvent* event);

    void tweakStrings();

private:
    Ui::LockedPopOver *ui;
};


