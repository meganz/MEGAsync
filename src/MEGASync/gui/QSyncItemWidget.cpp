#include "QSyncItemWidget.h"
#include "ui_QSyncItemWidget.h"
#include "Utilities.h"
#include "MegaApplication.h"
#include "megaapi.h"
#include "SyncInfo.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

QSyncItemWidget::QSyncItemWidget(int itemType, QWidget *parent) :
    QWidget(parent), itemType(itemType),
    ui(new Ui::QSyncItemWidget)
{
    ui->setupUi(this);

    optionsMenu = NULL;
    syncInfo = NULL;
    openDebris = NULL;
    deleteSync = NULL;
    isCacheAvailable = false;

    installEventFilter(this);

    configureSyncTypeUI(itemType);
}

void QSyncItemWidget::setText(const QString &path)
{
    fullPath = path;

    QString syncName {QFileInfo(fullPath).fileName()};
    if (syncName.isEmpty())
    {
        syncName = QDir::toNativeSeparators(fullPath);
    }
    syncName.remove(QChar::fromAscii(':')).remove(QDir::separator());
    ui->lSyncName->setText(syncName);
}

void QSyncItemWidget::setToolTip(const QString &tooltip)
{
    ui->lSyncName->setToolTip(tooltip);
}

QString QSyncItemWidget::text()
{
    return fullPath;
}

void QSyncItemWidget::localCacheAvailable(bool op)
{
    isCacheAvailable = op;
}

QSyncItemWidget::~QSyncItemWidget()
{
    //TODO: Ask UI team and apply colourful icons for open debris, info and delete syncs
    if (optionsMenu)
    {
        QList<QAction *> actions = optionsMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            optionsMenu->removeAction(actions[i]);
            delete actions[i];
        }
        delete optionsMenu;
    }
    optionsMenu = NULL;
    delete ui;
}

void QSyncItemWidget::on_bSyncOptions_clicked()
{
    if(!optionsMenu)
    {
        optionsMenu = new QMenu();
#ifdef __APPLE__
        optionsMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        optionsMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif
    }
    else
    {
        QList<QAction *> actions = optionsMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            optionsMenu->removeAction(actions[i]);
        }
    }

    //Add local cache item (if available)
    if (isCacheAvailable)
    {
        if (openDebris)
        {
            openDebris->deleteLater();
            openDebris = NULL;
        }

    #ifndef __APPLE__
        auto openLocalDebris{tr("Open Rubbish folder")};
    #else
        auto openLocalDebris{tr("Open .debris folder")};
    #endif
        openDebris = new MenuItemAction(openLocalDebris, QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
        connect(openDebris, SIGNAL(triggered()), this, SIGNAL(onOpenLocalCache()), Qt::QueuedConnection);
    }

    //Add sync info menu item
    if (syncInfo)
    {
        syncInfo->deleteLater();
        syncInfo = NULL;
    }

    syncInfo = new MenuItemAction(tr("Info"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
    connect(syncInfo, SIGNAL(triggered()), this, SIGNAL(onSyncInfo()), Qt::QueuedConnection);

    //Add deleteSync item
    if (deleteSync)
    {
        deleteSync->deleteLater();
        deleteSync = NULL;
    }

    deleteSync = new MenuItemAction(tr("Delete Sync"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
    connect(deleteSync, SIGNAL(triggered()), this, SIGNAL(onDeleteSync()), Qt::QueuedConnection);

    optionsMenu->addAction(syncInfo);

    if (isCacheAvailable)
    {
        optionsMenu->addAction(openDebris);
    }

    optionsMenu->addSeparator();
    optionsMenu->addAction(deleteSync);

    if (optionsMenu->isVisible())
    {
        optionsMenu->close();
    }

    QPoint point = ui->bSyncOptions->mapToGlobal(QPoint(ui->bSyncOptions->width(), ui->bSyncOptions->height() + 2));
    QPoint p = !point.isNull() ? point - QPoint(ui->bSyncOptions->width(), 0)
                             : QCursor::pos();
    optionsMenu->popup(p);

    ui->bReveal->hide();
}

void QSyncItemWidget::on_bReveal_clicked()
{
    switch (itemType)
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

