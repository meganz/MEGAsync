#include "QSyncItemWidget.h"
#include "ui_QSyncItemWidget.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "megaapi.h"


#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

QSyncItemWidget::QSyncItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSyncItemWidget)
{
    ui->setupUi(this);
    ui->bWarning->hide();
    error = 0;
}

void QSyncItemWidget::setPathAndName(const QString &path, const QString &syncName)
{
    mFullPath = path;
    ui->lSyncName->setText(syncName);
}

void QSyncItemWidget::setPathAndGuessName(const QString &path)
{
    mFullPath = path;

    QString syncName {QFileInfo(mFullPath).fileName()};
    if (syncName.isEmpty())
    {
        syncName = QDir::toNativeSeparators(mFullPath);
    }
    syncName.remove(QChar::fromAscii(':')).remove(QDir::separator());
    ui->lSyncName->setText(syncName);
}

void QSyncItemWidget::setToolTip(const QString &tooltip)
{
    ui->lSyncName->setToolTip(tooltip);
}

void QSyncItemWidget::setError(int error)
{
    this->error = error;

    if (error)
    {
        ui->bWarning->setToolTip(QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(error)));
        ui->bWarning->show();
    }
}

QString QSyncItemWidget::fullPath()
{
    return mFullPath;
}

QSyncItemWidget::~QSyncItemWidget()
{
    delete ui;
}
