#include "ScanningWidget.h"
#include "ui_ScanningWidget.h"

#include <QMovie>
#include "BlurredShadowEffect.h"
#include "Utilities.h"
#include "TransferMetaData.h"

ScanningWidget::ScanningWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::ScanningWidget)
{
    mUi->setupUi(this);
    mMovie = new QMovie(this);
    mMovie->setCacheMode(QMovie::CacheAll);

    qreal ratio = Utilities::getDevicePixelRatio();
    QString gifFile = (ratio < 2) ? QString::fromUtf8(":/animations/scanning.gif")
                                  : QString::fromUtf8(":/animations/scanning@2x.gif");
    mMovie->setFileName(gifFile);

    setRole(mUi->lStepTitle, "title");
    setRole(mUi->lStepDescription, "details");

    mUi->pBlockingStageCancel->setGraphicsEffect(CreateBlurredShadowEffect());
    connect(mUi->pBlockingStageCancel, &QPushButton::clicked,
            this, &ScanningWidget::onCancelClicked);

    mPreviousStage = mega::MegaTransfer::STATE_NONE;
}

ScanningWidget::~ScanningWidget()
{
    delete mUi;
    delete mMovie;
}

void ScanningWidget::show()
{
    startAnimation();

    mUi->pBlockingStageCancel->show();
    mUi->pBlockingStageCancel->setEnabled(true);
    mUi->lStepTitle->setText(tr("Scanning"));
    mUi->lStepDescription->setText(QString());
}

void ScanningWidget::hide()
{
    mMovie->stop();
    mUi->lAnimation->setMovie(nullptr);
}

void ScanningWidget::disableCancelButton()
{
    mUi->pBlockingStageCancel->setEnabled(false);
}

void ScanningWidget::updateAnimation()
{
    if(mMovie->state() == QMovie::Running)
    {
        mMovie->stop();
        mUi->lAnimation->setMovie(nullptr);
    }
    startAnimation();
}

void ScanningWidget::onReceiveStatusUpdate(const FolderTransferUpdateEvent &event)
{
    const auto metaData = TransferMetaDataContainer::getAppDataByAppData(event.appData.c_str());
    if (metaData && (metaData->getPendingFiles() + metaData->getFileTransfersOK()) > 0)
    {
        const auto addedTransfers = metaData->getPendingFiles() + metaData->getFileTransfersOK();
        mUi->lStepTitle->setText(tr("Adding transfers…"));
        mUi->lStepDescription->setText(tr("%1/%2").arg(addedTransfers).arg(event.filecount));
    }
    else
    {
        switch (event.stage)
        {
            case mega::MegaTransfer::STAGE_SCAN:
            {
                mUi->lStepTitle->setText(tr("Scanning"));
                mUi->lStepDescription->setText(buildScanDescription(event.foldercount, event.filecount));
                break;
            }
            case mega::MegaTransfer::STAGE_CREATE_TREE:
            {
                mUi->lStepTitle->setText(tr("Creating folders"));
                mUi->lStepDescription->setText(tr("%1/%2").arg(event.createdfoldercount).arg(event.foldercount));
                break;
            }
        }
    }
    mPreviousStage = event.stage;
}

void ScanningWidget::onCancelClicked()
{
    emit cancel();
}

void ScanningWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void ScanningWidget::startAnimation()
{
    if (mMovie->isValid())
    {
        mUi->lAnimation->setMovie(mMovie);
        mMovie->start();
    }
}

QString ScanningWidget::buildScanDescription(const uint32_t folderCount, const uint32_t fileCount)
{
    QString folderStr = tr("%n folder", "", folderCount);
    QString fileStr = tr("%n file", "", fileCount);
    return tr("found %1, %2").arg(folderStr, fileStr);
}

void ScanningWidget::setRole(QObject *object, const char *name)
{
    object->setProperty("role", QString::fromLatin1(name));
}

QString ScanningWidget::formattedNode(const QString &name)
{
    const QString quote = QString::fromLatin1("");
    const QChar ellipsis(0x2026);
    return quote + name + quote + ellipsis;
}
