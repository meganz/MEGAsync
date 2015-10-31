#include "InfoOverQuotaDialog.h"
#include "ui_InfoOverQuotaDialog.h"
#include "control/Utilities.h"
#include "MegaApplication.h"
#include <QPainter>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

InfoOverQuotaDialog::InfoOverQuotaDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoOverQuotaDialog)
{
    ui->setupUi(this);

    //Set window properties
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

#ifdef __APPLE__
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    preferences = Preferences::instance();

    this->app = app;
    megaApi = app->getMegaApi();

    QPixmap megaLogo(QString::fromUtf8(":/images/mega_logo.png"));
    QPixmap megaDisabledLogo(megaLogo.size());
    megaDisabledLogo.fill(Qt::transparent);
    QPainter pMegaLogo(&megaDisabledLogo);
    pMegaLogo.setBackgroundMode(Qt::TransparentMode);
    pMegaLogo.setBackground(QBrush(Qt::transparent));
    pMegaLogo.eraseRect(megaLogo.rect());
    pMegaLogo.setOpacity(0.3);
    pMegaLogo.drawPixmap(0, 0, megaLogo);
    pMegaLogo.end();
    QIcon iconMegaLogo;
    iconMegaLogo.addPixmap(megaDisabledLogo, QIcon::Disabled, QIcon::On);
    ui->lHeader->setIcon(iconMegaLogo);


    QPixmap syncFolder(QString::fromUtf8(":/images/tray_folder_ico.png"));
    QPixmap syncDisabledFolder(syncFolder.size());
    syncDisabledFolder.fill(Qt::transparent);
    QPainter pSyncFolder(&syncDisabledFolder);
    pSyncFolder.setBackgroundMode(Qt::TransparentMode);
    pSyncFolder.setBackground(QBrush(Qt::transparent));
    pSyncFolder.eraseRect(syncFolder.rect());
    pSyncFolder.setOpacity(0.3);
    pSyncFolder.drawPixmap(0, 0, syncFolder);
    pSyncFolder.end();
    QIcon iconSyncFolder;
    iconSyncFolder.addPixmap(syncDisabledFolder, QIcon::Disabled, QIcon::On);
    ui->bSyncFolder->setIcon(iconSyncFolder);


    ui->lDescDisabled->setText(QString::fromUtf8("<p style=\" line-height: 140%;\"><span style=\"font-size:14px;\">")
                               + ui->lDescDisabled->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                                                          .replace(QString::fromUtf8("[/A]"), QString::fromUtf8(" </font>"))
                                                                   + QString::fromUtf8("</span></p>"));
    setUsage();
}

InfoOverQuotaDialog::~InfoOverQuotaDialog()
{
    delete ui;
}

void InfoOverQuotaDialog::setUsage()
{
    if (!preferences->totalStorage())
    {
        return;
    }

    int percentage = ceil((100 * preferences->usedStorage()) / (double)preferences->totalStorage());
    ui->pUsage->setProgress(preferences->cloudDriveStorage(), preferences->rubbishStorage(),
                            preferences->inShareStorage(),preferences->inboxStorage(),preferences->totalStorage(),preferences->usedStorage());
    QString used = tr("%1 of %2").arg(QString::number(percentage).append(QString::fromAscii("%")))
            .arg(Utilities::getSizeString(preferences->totalStorage()));
    ui->lPercentageUsed->setText(used);
    ui->lTotalUsed->setText(tr("Usage: %1").arg(Utilities::getSizeString(preferences->usedStorage())));
}

void InfoOverQuotaDialog::on_bSettings_clicked()
{
    QPoint p = ui->bSettings->mapToGlobal(QPoint(ui->bSettings->width()-6, ui->bSettings->height()));

#ifdef __APPLE__
    QPointer<InfoOverQuotaDialog> iod = this;
#endif

    app->showTrayMenu(&p);

#ifdef __APPLE__
    if (!iod)
    {
        return;
    }

    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }
#endif
}

void InfoOverQuotaDialog::on_bUpgrade_clicked()
{
    megaApi->getSessionTransferURL("pro");
}

void InfoOverQuotaDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if (preferences->totalStorage())
        {
            setUsage();
        }

        ui->lDescDisabled->setText(QString::fromUtf8("<p style=\" line-height: 140%;\"><span style=\"font-size:14px;\">")
                                   + ui->lDescDisabled->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                                                              .replace(QString::fromUtf8("[/A]"), QString::fromUtf8(" </font>"))
                                                                       + QString::fromUtf8("</span></p>"));
    }
    QDialog::changeEvent(event);
}

void InfoOverQuotaDialog::on_bOfficialWeb_clicked()
{
    QString webUrl = QString::fromAscii("https://mega.nz/");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(webUrl));
}

#ifndef Q_OS_LINUX
void InfoOverQuotaDialog::on_bOfficialWebIcon_clicked()
{
    on_bOfficialWeb_clicked();
}
#endif
