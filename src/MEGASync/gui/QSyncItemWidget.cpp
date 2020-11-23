#include "QSyncItemWidget.h"
#include "ui_QSyncItemWidget.h"
#include "Utilities.h"
#include "MegaApplication.h"

QSyncItemWidget::QSyncItemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSyncItemWidget)
{
    ui->setupUi(this);

    optionsMenu = NULL;
    openFolder = NULL;
    openDebris = NULL;
    deleteSync = NULL;
    isCacheAvailable = false;
}

void QSyncItemWidget::setText(const QString &path)
{
    ui->lSyncName->setText(path);
}

void QSyncItemWidget::setToolTip(const QString &tooltip)
{
    ui->lSyncName->setToolTip(toolTip());
}

QString QSyncItemWidget::text()
{
    return ui->lSyncName->text();
}

void QSyncItemWidget::localCacheAvailable(bool op)
{
    isCacheAvailable = op;
}

QSyncItemWidget::~QSyncItemWidget()
{
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

    if (openFolder)
    {
        openFolder->deleteLater();
        openFolder = NULL;
    }

#ifndef __APPLE__
    openFolder = new MenuItemAction(tr("Open in file explorer"), QIcon(QString::fromAscii("://images/ico_quit_out.png")), QIcon(QString::fromAscii("://images/ico_quit_over.png")), true);
#else
    openFolder = new MenuItemAction(tr("Open in Finder"), QIcon(QString::fromAscii("://images/ico_quit_out.png")), QIcon(QString::fromAscii("://images/ico_quit_over.png")), true);
#endif
    connect(openFolder, SIGNAL(triggered()), this, SIGNAL(onOpenSync()), Qt::QueuedConnection);

    if (isCacheAvailable)
    {
        if (openDebris)
        {
            openDebris->deleteLater();
            openDebris = NULL;
        }

    #ifndef __APPLE__
        openDebris = new MenuItemAction(tr("Open Rubbish folder"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")), true);
    #else
        openDebris = new MenuItemAction(tr("Open .debris folder"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")), true);
    #endif
        connect(openDebris, SIGNAL(triggered()), this, SIGNAL(onOpenLocalCache()), Qt::QueuedConnection);
    }

    if (deleteSync)
    {
        deleteSync->deleteLater();
        deleteSync = NULL;
    }

#ifndef __APPLE__
    deleteSync = new MenuItemAction(tr("Delete Sync"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")), true);
#else
    deleteSync = new MenuItemAction(tr("Delete Sync"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")), true);
#endif
    connect(deleteSync, SIGNAL(triggered()), this, SIGNAL(onDeleteSync()), Qt::QueuedConnection);

    optionsMenu->addAction(openFolder);

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
}
