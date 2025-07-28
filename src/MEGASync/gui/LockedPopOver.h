#ifndef LOCKED_POP_OVER_H
#define LOCKED_POP_OVER_H

#include "ui_LockedPopOver.h"

#include <QDialog>

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
    bool event(QEvent* event) override;
    void showEvent(QShowEvent* event);

    void tweakStrings();

private:
    Ui::LockedPopOver *ui;
};
#endif
