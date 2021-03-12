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

    connect(Model::instance(), SIGNAL(syncStateChanged(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSetting>)));

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
    // when entering the item we trigger an update of the remote node path, to ensure we have an updated value
    if (event->type() == QEvent::Enter && mSyncRootHandle != mega::INVALID_HANDLE && mSyncSetting
            && mLastRemotePathCheck + 5000 < QDateTime::currentMSecsSinceEpoch() ) //only one path update every 5 secs
    {
        //queue an update of the sync remote node
        ThreadPoolSingleton::getInstance()->push([this]()
        {//thread pool function

            std::unique_ptr<char[]> np(MegaSyncApp->getMegaApi()->getNodePathByNodeHandle(mSyncSetting->getMegaHandle()));
            Model::instance()->updateMegaFolder(np ? QString::fromUtf8(np.get()) : QString(), mSyncSetting);

        });// end of thread pool function
        mLastRemotePathCheck = QDateTime::currentMSecsSinceEpoch();
    }
    return QWidget::event(event);
}

void QSyncItemWidget::onSyncStateChanged(std::shared_ptr<SyncSetting> syncSettings)
{
    if (mSyncSetting && syncSettings->getSyncID() == mSyncSetting->getSyncID())
    {
        ui->lSyncName->setToolTip(mSyncSetting->getMegaFolder());
    }
}

void QSyncItemWidget::setSyncSetting(const std::shared_ptr<SyncSetting> &value)
{
    mSyncSetting = value;
}
