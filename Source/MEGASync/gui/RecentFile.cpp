#include "RecentFile.h"
#include "ui_RecentFile.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "platform/Platform.h"

#include <QImageReader>

using namespace mega;

RecentFile::RecentFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecentFile)
{
    ui->setupUi(this);
    ui->lTime->setText(QString::fromUtf8(""));
    info.fileHandle = mega::INVALID_HANDLE;
    menu = NULL;
    getLinkDisabled = false;
    ui->pArrow->installEventFilter(this);
    update();
}

RecentFile::~RecentFile()
{
    delete ui;
}

void RecentFile::setFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey, long long time)
{
    info.fileName = fileName;
    info.fileHandle = fileHandle;
    info.localPath = localPath;
    info.nodeKey = nodeKey;
    info.dateTime = QDateTime::fromMSecsSinceEpoch(time);
}

void RecentFile::updateWidget()
{
    closeMenu();

    if(!info.fileName.length())
	{
        ui->lFileType->setText(QString());
        ui->lTime->setText(QString::fromAscii(""));
        ui->pArrow->installEventFilter(this);
        ui->pArrow->update();
        return;
	}

    if(info.fileName.compare(ui->lFileName->text()))
    {
        QFont f = ui->lFileName->font();
        QFontMetrics fm = QFontMetrics(f);
        ui->lFileName->setText(fm.elidedText(info.fileName, Qt::ElideRight,ui->lFileName->width()));

        QIcon icon;
        icon.addFile(Utilities::getExtensionPixmapMedium(info.fileName), QSize(), QIcon::Normal, QIcon::Off);

#ifndef Q_OS_LINUX
        ui->lFileType->setIcon(icon);
        ui->lFileType->setIconSize(QSize(48, 48));
#else
        ui->lFileType->setPixmap(icon.pixmap(QSize(48, 48)));
#endif
    }

    if(getLinkDisabled)
    {
        ui->pArrow->installEventFilter(this);
        ui->pArrow->update();
    }
    else
    {
        ui->pArrow->removeEventFilter(this);
        ui->pArrow->update();
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = info.dateTime.secsTo(now);
    if(secs < 2)
        ui->lTime->setText(tr("just now"));
    else if(secs < 60)
        ui->lTime->setText(tr("%1 seconds ago").arg(secs));
    else if(secs < 3600)
    {
        int minutes = secs/60;
        if(minutes == 1)
            ui->lTime->setText(tr("1 minute ago"));
        else
            ui->lTime->setText(tr("%1 minutes ago").arg(minutes));
    }
    else if(secs < 86400)
    {
        int hours = secs/3600;
        if(hours == 1)
            ui->lTime->setText(tr("1 hour ago"));
        else
            ui->lTime->setText(tr("%1 hours ago").arg(hours));
    }
    else if(secs < 2592000)
    {
        int days = secs/86400;
        if(days == 1)
            ui->lTime->setText(tr("1 day ago"));
        else
            ui->lTime->setText(tr("%1 days ago").arg(days));
    }
    else if(secs < 31536000)
    {
        int months = secs/2592000;
        if(months == 1)
            ui->lTime->setText(tr("1 month ago"));
        else
            ui->lTime->setText(tr("%1 months ago").arg(months));
    }
    else
    {
        int years = secs/31536000;
        if(years == 1)
            ui->lTime->setText(tr("1 year ago"));
        else
            ui->lTime->setText(tr("%1 years ago").arg(years));
    }
}

void RecentFile::closeMenu()
{
    if(menu)
        menu->close();
}

RecentFileInfo RecentFile::getFileInfo()
{
    return info;
}

void RecentFile::setFileInfo(RecentFileInfo info)
{
    this->info = info;
}

void RecentFile::disableGetLink(bool disable)
{
    this->getLinkDisabled = disable;
    updateWidget();
    update();
}

bool RecentFile::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

void RecentFile::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        updateWidget();

    QWidget::changeEvent(event);
}

void RecentFile::on_pArrow_clicked()
{
    if(getLinkDisabled)
        return;

    if(info.fileHandle != mega::INVALID_HANDLE)
        ((MegaApplication*)qApp)->copyFileLink(info.fileHandle, info.nodeKey);
}

void RecentFile::on_lFileType_customContextMenuRequested(const QPoint &pos)
{
    if(menu)
    {
        menu->close();
        return;
    }

    if(info.localPath.isEmpty() || !QFileInfo(info.localPath).exists()) return;

    menu = new QMenu();

#if (QT_VERSION == 0x050500) && defined(_WIN32)
    menu->installEventFilter(qApp);
#endif

#ifndef __APPLE__
    menu->setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif
    menu->addAction(tr("Open"), this, SLOT(openFile()));
    menu->addAction(tr("Show in folder"), this, SLOT(showInFolder()));
#ifdef WIN32
    menu->exec(this->mapToGlobal(pos));
#else
    menu->exec(this->mapToGlobal(QPoint(pos.x(), 0)));
#endif

    menu->deleteLater();
    menu = NULL;
}

void RecentFile::on_wText_customContextMenuRequested(const QPoint &pos)
{
    if(menu)
    {
        menu->close();
        return;
    }

    if(info.localPath.isEmpty() || !QFileInfo(info.localPath).exists()) return;

    menu = new QMenu();

#if (QT_VERSION == 0x050500) && defined(_WIN32)
    menu->installEventFilter(qApp);
#endif

#ifndef __APPLE__
    menu->setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif
    menu->addAction(tr("Open"), this, SLOT(openFile()));
    menu->addAction(tr("Show in folder"), this, SLOT(showInFolder()));
#ifdef WIN32
    menu->exec(this->mapToGlobal(pos));
#else
    menu->exec(this->mapToGlobal(QPoint(pos.x(), 0)));
#endif

    menu->deleteLater();
    menu = NULL;
}

void RecentFile::showInFolder()
{
    if(!info.localPath.isEmpty())
    {
        QWidget::window()->hide();
        Platform::showInFolder(info.localPath);
    }
}

void RecentFile::openFile()
{
    if(!info.localPath.isEmpty())
    {
        QWidget::window()->hide();
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.localPath));
    }
}
