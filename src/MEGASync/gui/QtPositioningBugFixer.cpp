#include "QtPositioningBugFixer.h"

#include <QTimer>

QtPositioningBugFixer::QtPositioningBugFixer(QDialog* _dialog)
    : dialogToFix(_dialog)
{
}

void QtPositioningBugFixer::onStartMove()
{
    dialogToFix->hide();
    originalFlags = dialogToFix->windowFlags();
    dialogToFix->setWindowFlags(Qt::Window);
}

void QtPositioningBugFixer::onEndMove()
{
    QTimer::singleShot(1, this, [this](){
        dialogToFix->setWindowFlags(originalFlags);
        dialogToFix->show();
    });
}
