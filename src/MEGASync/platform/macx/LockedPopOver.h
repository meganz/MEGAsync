#ifndef LOCKEDPOPOVER_H
#define LOCKEDPOPOVER_H

#include "ui_LockedPopOver.h"

#include <QMacNativeWidget>

namespace Ui {
class LockedPopOver;
}

class LockedPopOver : public QMacNativeWidget
{
    Q_OBJECT

public:
    LockedPopOver();

protected:
    void changeEvent(QEvent *event);
    void tweakStrings();

private:
    Ui::LockedPopOver m_ui;
};

#endif // LOCKEDPOPOVER_H
