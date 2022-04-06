#include "QSyncItemWidget.h"
#include "ui_QSyncItemWidget.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "megaapi.h"

#include <QtConcurrent/QtConcurrent>

constexpr auto CHK_RPATH_INTERVAL_MS (5000); // 5s

QSyncItemWidget::QSyncItemWidget(int itemType, QWidget* parent) :
    QWidget (parent),
    mUi (new Ui::QSyncItemWidget),
    mItemType (itemType),
    mError (0),
    mSelected (false),
    mLastRemotePathCheck (0),
    mNodesUpToDate (true)
{
    mUi->setupUi(this);
    installEventFilter(this);
    configureSyncTypeUI(itemType);

    connect(Model::instance(), &Model::syncStateChanged,
            this, &QSyncItemWidget::onSyncStateChanged);

    connect(MegaSyncApp, &MegaApplication::nodeMoved,
            this, &QSyncItemWidget::nodeChanged,
            Qt::DirectConnection); //direct connection for efficiency

    connect(MegaSyncApp, &MegaApplication::nodeAttributesChanged,
            this, &QSyncItemWidget::nodeChanged,
            Qt::DirectConnection);

    setSelected(false);
}

void QSyncItemWidget::setPath(const QString &path, const QString &syncName)
{
    mFullPath = path;
    mDisplayName = syncName;
    elidePathLabel();
}

void QSyncItemWidget::setPath(const QString &path)
{
    mFullPath = path;

    mDisplayName = QFileInfo(mFullPath).fileName();
    if (mDisplayName.isEmpty())
    {
        mDisplayName = QDir::toNativeSeparators(mFullPath);
    }

    mDisplayName.remove(QDir::separator());

    //If full sync mode ("/"), avoid empty display name
    if (mDisplayName.isEmpty())
    {
        mDisplayName.append(QLatin1Char('/'));
    }

    elidePathLabel();
}

void QSyncItemWidget::setToolTip(const QString &tooltip)
{
    if (mSyncRootHandle == mega::INVALID_HANDLE)
    {
        mUi->lSyncName->setToolTip(tooltip);
    }
}

QSyncItemWidget::~QSyncItemWidget()
{
    delete mUi;
}

void QSyncItemWidget::configureSyncTypeUI(int type) const
{
    mUi->lSyncState->setVisible(type == LOCAL_FOLDER);
}

void QSyncItemWidget::setError(int error)
{
    mError = error;

    if (error)
    {
        mUi->lSyncState->setStyleSheet(QString::fromLatin1("#lSyncState {border-image: url(\"://images/ic_sync_warning.png\")}"));
        mUi->lSyncState->setToolTip(QCoreApplication::translate("MegaSyncError",
                                                             mega::MegaSync::getMegaSyncErrorCode(
                                                                 error)));
    }
    else
    {
        mUi->lSyncState->setToolTip(QString());
        setSelected(mSelected);
    }

    elidePathLabel();
}

QString QSyncItemWidget::fullPath()
{
    return mFullPath;
}

void QSyncItemWidget::elidePathLabel()
{
    QFontMetrics metrics(mUi->lSyncName->fontMetrics());
    mUi->lSyncName->setText(metrics.elidedText(mDisplayName, Qt::ElideMiddle,
                                               mUi->lSyncName->width()));
}

void  QSyncItemWidget::resizeEvent(QResizeEvent *event)
{
    elidePathLabel();
    QWidget::resizeEvent(event);
}

void QSyncItemWidget::enterEvent(QEvent* event)
{
    // when entering the item we trigger an update of the remote node path,
    // to ensure we have an updated value
    if (!mNodesUpToDate
            && mSyncRootHandle != mega::INVALID_HANDLE
            && mSyncSetting
            && mLastRemotePathCheck + CHK_RPATH_INTERVAL_MS < QDateTime::currentMSecsSinceEpoch())
    {
        mNodesUpToDate = true; // to avoid further triggering updates, until some node changes

        //queue an update of the sync remote node
        ThreadPoolSingleton::getInstance()->push([this]()
        {//thread pool function

            auto megaApi (MegaSyncApp->getMegaApi());
            std::unique_ptr<char[]> np (megaApi->getNodePathByNodeHandle(
                                            mSyncSetting->getMegaHandle()));
            Model::instance()->updateMegaFolder(np ? QString::fromUtf8(np.get())
                                                   : QString(),
                                                mSyncSetting);
        });// end of thread pool function
        mLastRemotePathCheck = QDateTime::currentMSecsSinceEpoch();
    }
    return QWidget::enterEvent(event);
}

void QSyncItemWidget::onSyncStateChanged(std::shared_ptr<SyncSetting> syncSettings)
{
    if (mSyncSetting && syncSettings->getSyncID() == mSyncSetting->getSyncID())
    {
        mUi->lSyncName->setToolTip(mSyncSetting->getMegaFolder());
    }
}

void QSyncItemWidget::nodeChanged(mega::MegaHandle handle)
{
    Q_UNUSED(handle)
    // We could check if the handle corresponds to remote node or its ancestors
    // but for the shake of efficiency, instead of doing so for every node update,
    // we simply update the flag, so that we will update the path when hovering the element
    mNodesUpToDate = false;
}

void QSyncItemWidget::setSyncSetting(const std::shared_ptr<SyncSetting>& value)
{
    mSyncSetting = value;
}

void QSyncItemWidget::setSelected(bool selected)
{
    mSelected = selected;
    if (mSelected)
    {
        mUi->lSyncName->setStyleSheet(QString::fromLatin1("#lSyncName {color: white;}"));
        if(!mError)
            mUi->lSyncState->setStyleSheet(QString::fromLatin1("#lSyncState {border-image: url(\"://images/Item-sync-press.png\")}"));
    }
    else
    {
        mUi->lSyncName->setStyleSheet(QString::fromLatin1("#lSyncName {color: black;}"));
        if(!mError)
            mUi->lSyncState->setStyleSheet(QString::fromLatin1("#lSyncState {border-image: url(\"://images/Item-sync-rest.png\")}"));
    }
}

