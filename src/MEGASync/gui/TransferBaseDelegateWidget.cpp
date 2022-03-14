#include "TransferBaseDelegateWidget.h"
#include "QMegaMessageBox.h"

#include <QDebug>
#include <QPointer>

TransferBaseDelegateWidget::TransferBaseDelegateWidget(QWidget *parent)
    : QWidget(parent),
      mPreviousState(TransferData::TransferState::TRANSFER_NONE)
{

}

void TransferBaseDelegateWidget::updateUi(const QExplicitlySharedDataPointer<TransferData> data, int)
{
    if(mData != data || mData->mTag != data->mTag)
    {
        mPreviousState = TransferData::TransferState::TRANSFER_NONE;
        mData = data;
    }

    setType();
    setFileNameAndType();
    updateTransferState();

    mPreviousState = mData->mState;
}

bool TransferBaseDelegateWidget::stateHasChanged()
{
    if(mData && mData->mState != mPreviousState)
    {
        return true;
    }
    else
    {
        return false;
    }
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

void TransferBaseDelegateWidget::render(QPainter *painter, const QRegion &sourceRegion)
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
        button->setIconSize(QSize(24,24));
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

    auto message = tr("Are you sure you want to retry this transfer?");

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
