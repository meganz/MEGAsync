#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QRect>
#include <QTimer>

#include "InfoDialog.h"
#include "ui_InfoDialog.h"

#include "MegaApplication.h"

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    settingsDialog = NULL;
    this->app = app;

    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    this->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 500 - 2);

    ui->wRecent1->setFileName("filename_compressed.zip");
    ui->wRecent2->setFileName("filename_document.pdf");
    ui->wRecent3->setFileName("filename_image.png");

    ui->wTransfer1->setFileName("Photoshop_file_2.psd");
    ui->wTransfer1->setPercentage(76);
    ui->wTransfer1->setType(0);
    ui->wTransfer2->setFileName("Illustrator_file.ai");
    ui->wTransfer2->setPercentage(50);
    ui->wTransfer2->setType(1);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::startAnimation()
{
    ui->sActiveTransfers->setCurrentIndex(1);
    ui->wTransfer1->setPercentage(5);
    ui->wTransfer2->setPercentage(23);
    ui->pUsage->setProgress(20);
    timer->start(100);
    app->showSyncingIcon();
}

void InfoDialog::setUsage(int totalGB, int percentage)
{
    double usedGB = totalGB * percentage / 100.0;
    ui->pUsage->setProgress(percentage);
    QString used(QString::number(percentage));
    used += "% of " + QString::number(totalGB) + " GB";
    ui->lPercentageUsed->setText(used);

    QString usage("Usage: ");
    usage += QString::number(usedGB) + " GB";
    ui->lTotalUsed->setText(usage);
}

void InfoDialog::timerUpdate()
{
    int value1 = ui->wTransfer1->getPercentage();
    if(value1<100) ui->wTransfer1->setPercentage(value1+2);

    int value2 = ui->wTransfer2->getPercentage();
    if(value2<100) ui->wTransfer2->setPercentage(value2+1);

    int value3 = (value1+value2)/2;
    setUsage(50, value3);

    if(value1 == 100 && value2 == 100)
    {
        ui->sActiveTransfers->setCurrentIndex(0);
        timer->stop();
        app->showSyncedIcon();
    }
}

void InfoDialog::on_bSettings_clicked()
{
    if(settingsDialog) delete settingsDialog;
    settingsDialog = new SettingsDialog(app);
    settingsDialog->show();
}

void InfoDialog::on_bOfficialWeb_clicked()
{
    QString helpUrl("https://mega.co.nz/");
    QDesktopServices::openUrl(QUrl(helpUrl));
}

void InfoDialog::on_bSyncFolder_clicked()
{
    ui->sActiveTransfers->setCurrentIndex(!ui->sActiveTransfers->currentIndex());
}
