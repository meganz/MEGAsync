#include "QSyncItemWidget.h"
#include "QToolTip.h"
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
    mSyncName = syncName;
    elidePathLabel();
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

    mSyncName = syncName;
    elidePathLabel();
}

void QSyncItemWidget::setToolTip(const QString &tooltip)
{
    if (mSyncRootHandle == mega::INVALID_HANDLE)
    {
        ui->lSyncName->setToolTip(tooltip);
    }
    else
    {
        mOriginalPath = tooltip;
    }
}

void QSyncItemWidget::setError(int error)
{
    this->error = error;

    if (error)
    {
        ui->bWarning->setToolTip(QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(error)));
        ui->bWarning->show();
    }

    elidePathLabel();
}

QString QSyncItemWidget::fullPath()
{
    return mFullPath;
}

QSyncItemWidget::~QSyncItemWidget()
{
    delete ui;
}

void QSyncItemWidget::elidePathLabel()
{
    QFontMetrics metrics(ui->lSyncName->fontMetrics());
    ui->lSyncName->setText(metrics.elidedText(mSyncName, Qt::ElideMiddle, ui->lSyncName->width()));
}

void  QSyncItemWidget::resizeEvent(QResizeEvent *event)
{
    elidePathLabel();
    QWidget::resizeEvent(event);
}

bool QSyncItemWidget::event(QEvent* event)
{
    //qt doco: QWidget::event() is the main event handlerand receives all the widget's events. Normally, we recommend reimplementing one of the specialized event handlers instead of this function. But here we want to catch the QEvent::ToolTip events, and since these are rather rare, there exists no specific event handler. For that reason we reimplement the main event handler, and the first thing we need to do is to determine the event's type :

    if (event->type() == QEvent::ToolTip && mSyncRootHandle != mega::INVALID_HANDLE && app)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);

        std::unique_ptr<char[]> np(app->getMegaApi()->getNodePathByNodeHandle(mSyncRootHandle));
        if (np)
        {
            QToolTip::showText(helpEvent->globalPos(), QString::fromUtf8(np.get()));
            return true;
        }
        else
        {
            QToolTip::showText(helpEvent->globalPos(), QString::fromUtf8("Deleted: ") + mOriginalPath);
            return true;
        }
    }
    return QWidget::event(event);
}
