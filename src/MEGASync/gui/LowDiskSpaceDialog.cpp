#include "LowDiskSpaceDialog.h"
#include "ui_LowDiskSpaceDialog.h"

#include "BlurredShadowEffect.h"
#include "Utilities.h"

#include <QFileIconProvider>

LowDiskSpaceDialog::LowDiskSpaceDialog(qint64 neededSize, qint64 freeSize,
                                       qint64 driveSize, const QString& driveName,
                                       QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LowDiskSpaceDialog)
{
    ui->setupUi(this);
    setupShadowEffect();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto message = tr("There is not enough space on %1. You need an additional %2 to download these files.");
    ui->lExplanation->setText(message.arg(driveName, toString(neededSize-freeSize)));

    ui->lDiskName->setText(driveName);
    ui->lFreeSpace->setText(tr("Free space: %1").arg(toString(freeSize)));
    ui->lTotalSize->setText(tr("Total size: %1").arg(toString(driveSize)));

    connect(ui->bTryAgain, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->bCancel, &QPushButton::clicked, this, &QDialog::reject);

    mHighDpiResize.init(this);
}

LowDiskSpaceDialog::~LowDiskSpaceDialog()
{
    delete ui;
}

QString LowDiskSpaceDialog::toString(qint64 bytes)
{
    return Utilities::getSizeString((bytes > 0) ? bytes : 0);
}

void LowDiskSpaceDialog::setupShadowEffect()
{
#ifndef _WIN32
    ui->bTryAgain->setGraphicsEffect(CreateBlurredShadowEffect(QColor(54, 122, 246, 64), 1.0));
    ui->bCancel->setGraphicsEffect(CreateBlurredShadowEffect(1.0));
#endif
}

