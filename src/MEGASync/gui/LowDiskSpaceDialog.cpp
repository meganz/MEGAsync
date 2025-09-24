#include "LowDiskSpaceDialog.h"

#include "BlurredShadowEffect.h"
#include "ui_LowDiskSpaceDialog.h"
#include "Utilities.h"

#include <QFileIconProvider>

LowDiskSpaceDialog::LowDiskSpaceDialog(long long neededSize,
                                       long long freeSize,
                                       long long driveSize,
                                       const QString& driveName,
                                       QWidget* parent):
    QDialog(parent),
    mUi(new Ui::LowDiskSpaceDialog),
    mneededSize(neededSize),
    mfreeSize(freeSize),
    mdriveSize(driveSize),
    mdriveName(driveName)
{
    mUi->setupUi(this);

    updateStrings();

    connect(mUi->bTryAgain, &QPushButton::clicked, this, &QDialog::accept);
    connect(mUi->bCancel, &QPushButton::clicked, this, &QDialog::reject);
}

LowDiskSpaceDialog::~LowDiskSpaceDialog()
{
    delete mUi;
}

QString LowDiskSpaceDialog::toString(long long bytes)
{
    return Utilities::getSizeString((bytes > 0) ? bytes : 0);
}

void LowDiskSpaceDialog::updateStrings()
{
    auto message =
        tr("There is not enough space on %1. You need an additional %2 to download these files.");
    mUi->lExplanation->setText(message.arg(mdriveName, toString(mneededSize - mfreeSize)));

    mUi->lDiskName->setText(mdriveName);
    mUi->lFreeSpace->setText(tr("Free space: %1").arg(toString(mfreeSize)));
    mUi->lTotalSize->setText(tr("Total size: %1").arg(toString(mdriveSize)));
}

bool LowDiskSpaceDialog::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateStrings();
    }
    return QDialog::event(event);
}

