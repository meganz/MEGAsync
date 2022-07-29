#include "TransferBaseDelegateWidget.h"
#include "QMegaMessageBox.h"
#include <MegaTransferView.h>

#include <QPointer>

const QString TransferBaseDelegateWidget::STATE_STARTING = QString::fromUtf8(QT_TR_NOOP("Starting…"));
const QString TransferBaseDelegateWidget::STATE_RETRYING = QLatin1String(QT_TR_NOOP("Retrying"));
const QString TransferBaseDelegateWidget::STATE_UPLOADING = QString::fromUtf8(QT_TR_NOOP("Uploading…"));
const QString TransferBaseDelegateWidget::STATE_DOWNLOADING = QString::fromUtf8(QT_TR_NOOP("Downloading…"));
const QString TransferBaseDelegateWidget::STATE_SYNCING = QString::fromUtf8(QT_TR_NOOP("Syncing…"));
const QString TransferBaseDelegateWidget::STATE_COMPLETING = QLatin1String(QT_TR_NOOP("Completing"));
const QString TransferBaseDelegateWidget::STATE_COMPLETED = QLatin1String(QT_TR_NOOP("Completed"));
const QString TransferBaseDelegateWidget::STATE_PAUSED = QLatin1String(QT_TR_NOOP("Paused"));
const QString TransferBaseDelegateWidget::STATE_FAILED = QLatin1String(QT_TR_NOOP("Failed"));
const QString TransferBaseDelegateWidget::STATE_INQUEUE = QLatin1String(QT_TR_NOOP("In queue"));
const QString TransferBaseDelegateWidget::STATE_INQUEUE_PARENTHESIS = QLatin1String(QT_TR_NOOP("(in queue)"));
const QString TransferBaseDelegateWidget::STATE_RETRY = QLatin1String(QT_TR_NOOP("Retry"));
const QString TransferBaseDelegateWidget::STATE_OUT_OF_STORAGE_SPACE = QLatin1String(QT_TR_NOOP("Out of storage space"));
const QString TransferBaseDelegateWidget::STATE_OUT_OF_TRANSFER_QUOTA = QLatin1String(QT_TR_NOOP("Out of transfer quota"));

TransferBaseDelegateWidget::TransferBaseDelegateWidget(QWidget *parent)
    : QWidget(parent),
      mPreviousState(TransferData::TransferState::TRANSFER_NONE)
{

}

void TransferBaseDelegateWidget::updateUi(const QExplicitlySharedDataPointer<TransferData> transferData, int)
{
    if(!mData || mData->mTag != transferData->mTag)
    {
        mPreviousState = TransferData::TransferState::TRANSFER_NONE;
    }

    mData = transferData;

    setType();
    setFileNameAndType();
    updateTransferState();

    mPreviousState = mData->getState();
}

bool TransferBaseDelegateWidget::stateHasChanged()
{
    return mPreviousState != mData->getState();
}

QExplicitlySharedDataPointer<TransferData> TransferBaseDelegateWidget::getData()
{
    return mData;
}

QModelIndex TransferBaseDelegateWidget::getCurrentIndex() const
{
    return mCurrentIndex;
}

void TransferBaseDelegateWidget::setCurrentIndex(const QModelIndex &currentIndex)
{
    mCurrentIndex = currentIndex;
}

void TransferBaseDelegateWidget::render(const QStyleOptionViewItem&, QPainter *painter, const QRegion &sourceRegion)
{
    QWidget::render(painter,QPoint(0,0),sourceRegion);
}

bool TransferBaseDelegateWidget::setActionTransferIcon(QToolButton *button, const QString &iconName)
{
    bool update(false);

    auto oldIconName = mLastActionTransferIconName.value(button, QString());

    if (oldIconName.isEmpty() || oldIconName != iconName)
    {
        button->setIcon(Utilities::getCachedPixmap(iconName));
        mLastActionTransferIconName.insert(button, iconName);

        update = true;
    }

    return update;
}

bool TransferBaseDelegateWidget::isMouseHoverInAction(QToolButton *button, const QPoint& mousePos)
{   
    if(button->testAttribute(Qt::WA_WState_Hidden))
    {
        return false;
    }

    auto actionGlobalPos = button->mapTo(this, QPoint(0,0));
    QRect actionGeometry(actionGlobalPos, button->size());

    return actionGeometry.contains(mousePos);
}

void TransferBaseDelegateWidget::onRetryTransfer()
{
    QPointer<TransferBaseDelegateWidget> dialog = QPointer<TransferBaseDelegateWidget>(this);

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             MegaTransferView::retryAskActionText(1),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit retryTransfer();
}
