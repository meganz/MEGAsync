#include "TransferBaseDelegateWidget.h"
#include "QMegaMessageBox.h"

#include <QPointer>

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

    auto message = tr("Retry transfer?" , "", 1);

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             message,
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit retryTransfer();
}
