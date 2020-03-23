#include "LockedPopOver.h"
#include "ui_LockedPopOver.h"

#include <QMouseEvent>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

LockedPopOver::LockedPopOver(QWidget *parent) :
    ui(new Ui::LockedPopOver)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
}


void LockedPopOver::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
