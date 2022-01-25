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
