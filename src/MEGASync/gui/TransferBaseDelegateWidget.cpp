#include "TransferBaseDelegateWidget.h"

#include <QDebug>

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
