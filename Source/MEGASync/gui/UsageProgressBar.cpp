#include "UsageProgressBar.h"
#include "ui_UsageProgressBar.h"
#include "math.h"

UsageProgressBar::UsageProgressBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UsageProgressBar)
{
    ui->setupUi(this);
    ui->wCloud->resize(0, ui->wCloud->height());
    ui->wRubbish->resize(0, ui->wRubbish->height());
    ui->wShares->resize(0, ui->wShares->height());
    ui->wInbox->resize(0, ui->wInbox->height());
}

UsageProgressBar::~UsageProgressBar()
{
    delete ui;
}

void UsageProgressBar::setProgress(long long valueCloud,long long valueRubbish, long long valueShares, long long valueInbox, long long totalBytes, long long totalUsed)
{

    //Reset default style
    ui->wCloud->setStyleSheet(QString::fromUtf8(""));
    ui->wRubbish->setStyleSheet(QString::fromUtf8(""));
    ui->wShares->setStyleSheet(QString::fromUtf8(""));
    ui->wInbox->setStyleSheet(QString::fromUtf8(""));

    /*if(valueCloud + valueRubbish + valueShares + valueInbox == totalUsed)
    {
        int value = ceil((100 * totalUsed) / (double)totalBytes);
        if(value>100) value = 100;
        if(value<5) value=5;

        ui->wCloud->resize(value*3.6, ui->wCloud->height());
        if(value<97)
        {
            ui->wCloud->setStyleSheet(QString::fromAscii("background-color: #78B241;"));
        }
        else
        {
            ui->wCloud->setStyleSheet(QString::fromAscii("background-color: #d90007;border-radius: 6px;"));
        }

    }else
    {*/
        int percentageCloud = ceil(1000*valueCloud/(double)totalBytes);
        int percentageRubbish = ceil(1000*valueRubbish/(double)totalBytes)+percentageCloud;
        int percentageShares = ceil(1000*valueShares/(double)totalBytes)+percentageRubbish;
        int percentageinbox = ceil(1000*valueInbox/(double)totalBytes)+percentageShares;

        if(percentageinbox>1000)
        {
            percentageCloud = percentageRubbish = percentageShares = percentageinbox = 993;
            ui->wCloud->setStyleSheet(QString::fromAscii("QWidget{background-color: #d90007;border-radius: 6px;}"));
            ui->wRubbish->setStyleSheet(QString::fromAscii("border-radius: 6px;"));
            ui->wShares->setStyleSheet(QString::fromAscii("border-radius: 6px;"));
            ui->wInbox->setStyleSheet(QString::fromAscii("border-radius: 6px;"));
        }

        if(percentageinbox<35)
        {
            percentageCloud = percentageRubbish = percentageShares = percentageinbox = 35;
        }

        ui->wCloud->resize(percentageCloud*0.36, ui->wInbox->height());
        ui->wRubbish->resize(percentageRubbish*0.36, ui->wRubbish->height());
        ui->wShares->resize(percentageShares*0.36, ui->wShares->height());
        ui->wInbox->resize(percentageinbox*0.36, ui->wCloud->height());

    /*}*/

}
