#include "MegaTransferDelegate.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QToolTip>
#include "control/Utilities.h"
#include "Preferences.h"
#include <InfoDialogTransfersWidget.h>
#include <TransferItem2.h>

#include "gui/QMegaMessageBox.h"
#include "megaapi.h"
#include "QTransfersModel.h"
#include "MegaApplication.h"
#include "platform/Platform.h"

using namespace mega;

MegaTransferDelegate::MegaTransferDelegate(InfoDialogCurrentTransfersProxyModel *model, QObject *parent)
    : QStyledItemDelegate(parent), mModel(model)
{
    mTransferItems.setMaxCost(16);
}

void MegaTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QColor(247, 247, 247));
        }

        auto transferItem = (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
        auto tData = transferItem.getTransferData();

        if(!tData)
        {
            return;
        }

        int tag = tData->mTag;
        bool isSyncTransfer = tData->mType.setFlag(TransferData::TransferType::TRANSFER_SYNC);
        TransferItem *ti = mTransferItems[tag];
        if (!ti)
        {
            ti = new CustomTransferItem();
            ti->setTransferTag(tag);
            mTransferItems.insert(tag, ti);

            //Check if transfer finishes while the account was blocked, in order to provide the right context for failed error
            bool blockedTransfer = static_cast<MegaApplication*>(qApp)->finishedTransfersWhileBlocked(tData->mTag);
            if (blockedTransfer)
            {
                ti->setTransferFinishedWhileBlocked(blockedTransfer);
                static_cast<MegaApplication*>(qApp)->removeFinishedBlockedTransfer(tData->mTag);
            }

            ti->setType(tData->mType, isSyncTransfer);
            ti->setFileName(tData->mFilename);

        }

        // Get http speed, which reports speed changes faster than the transfer.
        auto httpSpeed (static_cast<unsigned long long>(MegaSyncApp->getMegaApi()->getCurrentSpeed((tData->mType & TransferData::TYPE_MASK) >> 1)));
        ti->setSpeed(std::min(tData->mSpeed, httpSpeed), tData->mMeanSpeed);

        ti->setTransferredBytes(tData->mTransferredBytes, !isSyncTransfer);
        ti->setPriority(tData->mPriority);
        ti->setTotalSize(tData->mTotalSize);
        ti->setTransferState(tData->mState);

        int tError = tData->mErrorCode;
        if (tError != MegaError::API_OK)
        {
            ti->setTransferError(tError,  tData->mErrorValue);
        }

        if (ti->isTransferFinished())
        {
            if (!ti->getFinishedTime())
            {
                ti->setFinishedTime(tData->mFinishedTime);
                ti->updateFinishedTime(); // applies styles which can be slow - just do it when the finished time changes
            }

            if (tData->mPublicNode)
            {
               ti->setIsLinkAvailable(true);

               if (tData->mNodeAccess != mega::MegaShare::ACCESS_UNKNOWN)
               {
                   ti->setNodeAccess(tData->mNodeAccess);
               }
            }
        }

        else
        {
            if (!ti->isTransferFinished())
            {
                ti->updateTransfer();
            }
            else
            {
                ti->updateFinishedTime();
            }
        }

        Preferences *preferences = Preferences::instance();
        if (tData->mType.setFlag(TransferData::TransferType::TRANSFER_DOWNLOAD))
        {
            if (preferences->getDownloadsPaused())
            {
                ti->setStateLabel(tr("PAUSED"));
            }
            else
            {
                ti->updateAnimation();
            }
        }
        else if (tData->mType.setFlag(TransferData::TransferType::TRANSFER_UPLOAD))
        {
            if (preferences->getUploadsPaused())
            {
                ti->setStateLabel(tr("PAUSED"));
            }
            else
            {
                ti->updateAnimation();
            }
        }

        painter->save();
        painter->translate(option.rect.topLeft());

        ti->resize(option.rect.width(), option.rect.height());

        ti->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));
        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaTransferDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {
        return QSize(400, 60);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

void MegaTransferDelegate::processCancel(int tag)
{
    QMessageBox warning;
    HighDpiResize hDpiResizer(&warning);
    warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
    warning.setText(tr("Are you sure you want to cancel this transfer?"));
    warning.setIcon(QMessageBox::Warning);
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    warning.setDefaultButton(QMessageBox::No);
    int result = warning.exec();
    if (result == QMessageBox::Yes)
    {
        static_cast<MegaApplication*>(qApp)->getMegaApi()->cancelTransferByTag(tag);
    }
}

bool MegaTransferDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (QEvent::MouseButtonPress ==  event->type())
    {
        auto transferItem = (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
        auto tData = transferItem.getTransferData();

        if(tData)
        {
            int tag = tData->mTag;
            TransferItem *item = mTransferItems[tag];
            if (!item)
            {
                return true;
            }

            if (item->cancelButtonClicked(((QMouseEvent *)event)->pos() - option.rect.topLeft()))
            {
                processCancel(tag);
                return true; // click consumed
            }
            else if (item && item->checkIsInsideButton(((QMouseEvent *)event)->pos() - option.rect.topLeft(), TransferItem::ACTION_BUTTON))
            {
                 mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();
                 const auto &ti = mTransferItems[tag];
                 if (!ti->getIsLinkAvailable() && !ti->getTransferError())
                 {
                     processShowInFolder(index);
                 }
                 else if (MegaTransfer *transfer = megaApi->getTransferByTag(tag))
                 {
                     if (!transfer->getLastError().getErrorCode())
                     {
                         QList<MegaHandle> exportList;
                         QStringList linkList;
                         MegaNode *node = transfer->getPublicMegaNode();
                         if (!node || !node->isPublic())
                         {
                             exportList.push_back(transfer->getNodeHandle());
                         }
                         else
                         {
                             char *handle = node->getBase64Handle();
                             char *key = node->getBase64Key();
                             if (handle && key)
                             {
                                 QString link = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2")
                                         .arg(QString::fromUtf8(handle)).arg(QString::fromUtf8(key));
                                 linkList.append(link);
                             }
                             delete [] handle;
                             delete [] key;
                         }
                         delete node;
                         if (exportList.size() || linkList.size())
                         {
                             ((MegaApplication*)qApp)->exportNodes(exportList, linkList);
                         }
                     }
                     else
                     {
                         ((MegaApplication*)qApp)->getMegaApi()->retryTransfer(transfer);
                     }

                     delete transfer;
                 }

                 return true; // click consumed
            }
            else if (item && item->checkIsInsideButton(((QMouseEvent *)event)->pos() - option.rect.topLeft(), TransferItem::SHOW_IN_FOLDER_BUTTON))
            {
                 processShowInFolder(index);
                 return true; // click consumed
            }
        }
    }

    return QAbstractItemDelegate::editorEvent(event, mModel, option, index);
}

bool MegaTransferDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip)
    {
        auto transferItem = (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
        auto tData = transferItem.getTransferData();

        if(!tData)
        {
            return false;
        }

        int tag = index.internalId();
        TransferItem *item = mTransferItems[tag];
        if (item)
        {
            if (item->checkIsInsideButton(event->pos() - option.rect.topLeft(), TransferItem::ACTION_BUTTON))
            {
                const auto &ti = mTransferItems[tag];
                if (!ti->getIsLinkAvailable() && !ti->getTransferError())
                {
                    QToolTip::showText(event->globalPos(), tr("Show in folder"));
                }
                else
                {
                    if (tData->mErrorCode == 0)
                    {
                        QToolTip::showText(event->globalPos(), tr("Get link"));
                    }
                    else
                    {
                        QToolTip::showText(event->globalPos(), tr("Retry"));
                    }
                    return true;
                }

            }
            else if (item->checkIsInsideButton(event->pos() - option.rect.topLeft(), TransferItem::SHOW_IN_FOLDER_BUTTON))
            {
                QToolTip::showText(event->globalPos(), tr("Show in folder"));
                return true;
            }

            QString fileName = item->getFileName();
            if (fileName != item->getTransferName())
            {
                QToolTip::showText(event->globalPos(), fileName);
                return true;
            }
        }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

void MegaTransferDelegate::processShowInFolder(const QModelIndex &index)
{
    auto transferItem = (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
    auto tData = transferItem.getTransferData();
    if (tData && tData->mState == TransferData::TransferState::TRANSFER_COMPLETED
                 && !tData->mPath.isEmpty())
    {
        QString localPath = tData->mPath;
        #ifdef WIN32
        if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            localPath = localPath.mid(4);
        }
        #endif
        Platform::showInFolder(localPath);
    }
}
