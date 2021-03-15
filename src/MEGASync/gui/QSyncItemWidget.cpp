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


    connect(MegaSyncApp, SIGNAL(nodeMoved(mega::MegaHandle)),
            this, SLOT(nodeChanged(mega::MegaHandle)), Qt::DirectConnection); //direct connection for efficiency
    connect(MegaSyncApp, SIGNAL(nodeAttributesChanged(mega::MegaHandle)),
            this, SLOT(nodeChanged(mega::MegaHandle)), Qt::DirectConnection);

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
    if (event->type() == QEvent::Enter && !mNodesUpToDate && mSyncRootHandle != mega::INVALID_HANDLE && mSyncSetting
            && mLastRemotePathCheck + 5000 < QDateTime::currentMSecsSinceEpoch() ) //only one path update every 5 secs
    {
        mNodesUpToDate = true; // to avoid further triggering updates, until some node changes

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

void QSyncItemWidget::nodeChanged(mega::MegaHandle handle)
{
    // We could check if the handle corresponds to remote node or its ancestors
    // but for the shake of efficiency, instead of doing so for every node update,
    // we simply update the flag, so that we will update the path when hovering the element
    mNodesUpToDate = false;
}

void QSyncItemWidget::setSyncSetting(const std::shared_ptr<SyncSetting> &value)
{
    mSyncSetting = value;
}
