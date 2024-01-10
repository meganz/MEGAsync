#include "BackupsQmlDialog.h"

BackupsQmlDialog::BackupsQmlDialog(QWindow *parent)
    : QmlDialog(parent)
{
    setIcon(QIcon(QString::fromUtf8(":/images/icons/ico_backup.png")));
}
