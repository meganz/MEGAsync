#include "QtPositioningBugFixer.h"

#include <QTimer>

QtPositioningBugFixer::QtPositioningBugFixer(QDialog* _dialog)
    : dialogToFix(_dialog)
{
    applyFix = isFixApplied();
}

void QtPositioningBugFixer::onStartMove()
{
    if (applyFix)
    {
        originalFlags = dialogToFix->windowFlags();
        dialogToFix->hide();
        dialogToFix->setWindowFlag(Qt::Window);
    }
}

void QtPositioningBugFixer::onEndMove()
{
    if (applyFix)
    {
        QTimer::singleShot(1, this, [this](){
            dialogToFix->setWindowFlags(originalFlags);
            dialogToFix->show();
        });
    }
}

bool QtPositioningBugFixer::isFixApplied()
{
#ifdef __linux__
    QByteArray currentEnv = qgetenv("XDG_CURRENT_DESKTOP");
    return currentEnv.contains("GNOME");
#else
    return false;
#endif
}
