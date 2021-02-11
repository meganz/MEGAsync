#include "QSyncItemWidget.h"
#include "ui_QSyncItemWidget.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "megaapi.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

QSyncItemWidget::QSyncItemWidget(int itemType, QWidget *parent) :
    QWidget(parent), mItemType(itemType),
    ui(new Ui::QSyncItemWidget)
{
    ui->setupUi(this);

    mOptionsMenu = NULL;
    mSyncInfo = NULL;
    mOpenDebris = NULL;
    mDeleteSync = NULL;
    mIsCacheAvailable = false;
    mError = 0;

    QSizePolicy spRetain = ui->bReveal->sizePolicy();
    spRetain.setRetainSizeWhenHidden(true);
    ui->bReveal->setSizePolicy(spRetain);

    installEventFilter(this);
    configureSyncTypeUI(itemType);
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
    ui->lSyncName->setToolTip(tooltip);
}

QSyncItemWidget::~QSyncItemWidget()
{
    //TODO: Ask UI team and apply colourful icons for open debris, info and delete syncs
    if (mOptionsMenu)
    {
        QList<QAction *> actions = mOptionsMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            mOptionsMenu->removeAction(actions[i]);
            delete actions[i];
        }
        delete mOptionsMenu;
    }
    mOptionsMenu = NULL;
    delete ui;
}

void QSyncItemWidget::on_bSyncOptions_clicked()
{
    if(!mOptionsMenu)
    {
        mOptionsMenu = new QMenu();
#ifdef __APPLE__
        mOptionsMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        mOptionsMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif
    }
    else
    {
        QList<QAction *> actions = mOptionsMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            mOptionsMenu->removeAction(actions[i]);
        }
    }

    //Add local cache item (if available)
    if (mIsCacheAvailable)
    {
        if (mOpenDebris)
        {
            mOpenDebris->deleteLater();
            mOpenDebris = NULL;
        }

    #ifndef __APPLE__
        auto openLocalDebris{tr("Open Rubbish folder")};
    #else
        auto openLocalDebris{tr("Open .debris folder")};
    #endif
        mOpenDebris = new MenuItemAction(openLocalDebris, QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
        connect(mOpenDebris, SIGNAL(triggered()), this, SIGNAL(onOpenLocalCache()), Qt::QueuedConnection);
    }

    //Add sync info menu item
    if (mSyncInfo)
    {
        mSyncInfo->deleteLater();
        mSyncInfo = NULL;
    }

    mSyncInfo = new MenuItemAction(tr("Info"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
    connect(mSyncInfo, SIGNAL(triggered()), this, SIGNAL(onSyncInfo()), Qt::QueuedConnection);

    //Add deleteSync item
    if (mDeleteSync)
    {
        mDeleteSync->deleteLater();
        mDeleteSync = NULL;
    }

    mDeleteSync = new MenuItemAction(tr("Delete Sync"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
    connect(mDeleteSync, SIGNAL(triggered()), this, SIGNAL(onDeleteSync()), Qt::QueuedConnection);

    mOptionsMenu->addAction(mSyncInfo);

    if (mIsCacheAvailable)
    {
        mOptionsMenu->addAction(mOpenDebris);
    }

    mOptionsMenu->addSeparator();
    mOptionsMenu->addAction(mDeleteSync);

    if (mOptionsMenu->isVisible())
    {
        mOptionsMenu->close();
    }

    QPoint point = ui->bSyncOptions->mapToGlobal(QPoint(ui->bSyncOptions->width(), ui->bSyncOptions->height() + 2));
    QPoint p = !point.isNull() ? point - QPoint(ui->bSyncOptions->width(), 0)
                             : QCursor::pos();
    mOptionsMenu->popup(p);

    ui->bReveal->hide();
}

void QSyncItemWidget::on_bReveal_clicked()
{
    //TODO: Path is not longer presetn at lSyncName, so we need to figure out how to get that in order to open it
    // both local or remote.
    switch (mItemType)
    {
        case LOCAL_FOLDER:
        {
            QString localFolderPath {ui->lSyncName->text()};
            QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
        }
        break;
        case REMOTE_FOLDER:
        {
            QString megaFolderPath {ui->lSyncName->text()};
            const auto megaApp{static_cast<MegaApplication*>(qApp)};
            mega::MegaNode *node = megaApp->getMegaApi()->getNodeByPath(megaFolderPath.toUtf8().constData());
            if (node)
            {
                QtConcurrent::run(QDesktopServices::openUrl,
                                  QUrl(QString::fromUtf8("mega://#fm/%1").arg(QString::fromUtf8(node->getBase64Handle()))));
                delete node;
            }
        }
        break;
        default:
        break;
    }

    ui->bReveal->hide();
}

bool QSyncItemWidget::getIsCacheAvailable() const
{
    return mIsCacheAvailable;
}

void QSyncItemWidget::setIsCacheAvailable(bool value)
{
    mIsCacheAvailable = value;
}

void QSyncItemWidget::configureSyncTypeUI(int type) const
{
    switch (type)
    {
    case LOCAL_FOLDER:
    {
        ui->bSyncState->show();
        ui->bSyncOptions->hide();
        }
        break;
        case REMOTE_FOLDER:
        {
            ui->bSyncState->hide();
            ui->bSyncOptions->show();
        }
        break;
        default:
        break;
    }

    ui->bReveal->hide();
}

bool QSyncItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        ui->bReveal->show();
    }

    if (event->type() == QEvent::Leave)
    {
        ui->bReveal->hide();
    }

    return QWidget::eventFilter(obj,event);
}

void QSyncItemWidget::setError(int error)
{
    this->mError = error;

    if (error)
    {
        ui->bSyncState->setToolTip(QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(error)));
    }

    elidePathLabel();
}

QString QSyncItemWidget::fullPath()
{
    return mFullPath;
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
