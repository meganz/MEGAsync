#include "MegaTransferDelegate2.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "gui/QMegaMessageBox.h"
#include "megaapi.h"
#include "QTransfersModel.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "TransferManagerItem2.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QToolTip>

using namespace mega;

MegaTransferDelegate2::MegaTransferDelegate2(QTransfersModel2 *model, QObject *parent)
    : QStyledItemDelegate(parent),
      mModel(model)
{

}

void MegaTransferDelegate2::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    if (index.data().canConvert<TransferDataRow>())
    {
        TransferDataRow transferData = qvariant_cast<TransferDataRow>(index.data());

        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QColor(247, 247, 247));
        }       

        TransferManagerItem2* tmi = new TransferManagerItem2(transferData, this);

        tmi->paint(painter, option.rect);

        painter->save();
        painter->translate(option.rect.topLeft());


        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaTransferDelegate2::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{

        return QStyledItemDelegate::sizeHint(option, index);

}

void MegaTransferDelegate2::processCancel(int tag)
{
    if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
    {
        model->removeTransferByTag(tag);
    }
    else
    {
        QPointer<QTransfersModel> modelPointer = model;

        QMessageBox warning;
        HighDpiResize hDpiResizer(&warning);
        warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
        warning.setText(tr("Are you sure you want to cancel this transfer?"));
        warning.setIcon(QMessageBox::Warning);
        warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        warning.setDefaultButton(QMessageBox::No);
        int result = warning.exec();
        if (modelPointer && result == QMessageBox::Yes)
        {
            model->megaApi->cancelTransferByTag(tag);
        }
    }
}

bool MegaTransferDelegate2::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (QEvent::MouseButtonPress ==  event->type())
    {
        int tag = index.internalId();
        TransferItem *item = model->transferItems[tag];
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
            int modelType = model->getModelType();
            if (modelType == QTransfersModel::TYPE_FINISHED
                    || modelType == QTransfersModel::TYPE_CUSTOM_TRANSFERS)
            {
                const auto &ti = model->transferItems[tag];
                if (modelType == QTransfersModel::TYPE_CUSTOM_TRANSFERS && !ti->getIsLinkAvailable() && !ti->getTransferError())
                {
                    processShowInFolder(tag);
                }
                else if (MegaTransfer *transfer = model->getTransferByTag(tag) )
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
             }
             return true; // click consumed
        }
        else if (item && item->checkIsInsideButton(((QMouseEvent *)event)->pos() - option.rect.topLeft(), TransferItem::SHOW_IN_FOLDER_BUTTON))
        {
            if (model->getModelType() == QTransfersModel::TYPE_CUSTOM_TRANSFERS)
            {
                processShowInFolder(tag);
                return true; // click consumed
            }
            // we are not consuming the click; fall through to do the usual thing (of selecting the clicked row)
        }
    }
    else if (QEvent::MouseButtonDblClick == event->type() && model->getModelType() == QTransfersModel::TYPE_FINISHED)
    {
        int tag = index.internalId();
        processShowInFolder(tag);
        return true; // double-click consumed
    }

    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

bool MegaTransferDelegate2::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip)
    {
        int tag = index.internalId();
        TransferItem *item = model->transferItems[tag];
        if (item)
        {
            if (item->checkIsInsideButton(event->pos() - option.rect.topLeft(), TransferItem::ACTION_BUTTON))
            {
                int modelType = model->getModelType();

                if (modelType == QTransfersModel::TYPE_CUSTOM_TRANSFERS)
                {
                    const auto &ti = model->transferItems[tag];

                    if (!ti->getIsLinkAvailable() && !ti->getTransferError())
                    {
                        QToolTip::showText(event->globalPos(), tr("Show in folder"));
                    }
                    else if (MegaTransfer *transfer = model->getTransferByTag(tag) )
                    {
                        if (!transfer->getLastError().getErrorCode())
                        {
                            QToolTip::showText(event->globalPos(), tr("Get link"));
                        }
                        else
                        {
                            QToolTip::showText(event->globalPos(), tr("Retry"));
                        }
                        delete transfer;
                        return true;
                    }
                }
            }
            else if (item->checkIsInsideButton(event->pos() - option.rect.topLeft(), TransferItem::SHOW_IN_FOLDER_BUTTON))
            {
                int modelType = model->getModelType();
                if (modelType == QTransfersModel::TYPE_CUSTOM_TRANSFERS)
                {
                    QToolTip::showText(event->globalPos(), tr("Show in folder"));
                    return true;
                }
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

void MegaTransferDelegate2::processShowInFolder(int tag)
{
    MegaTransfer *transfer = NULL;
    transfer = model->getTransferByTag(tag);
    if (transfer && transfer->getState() == MegaTransfer::STATE_COMPLETED
                 && transfer->getPath())
    {
        QString localPath = QString::fromUtf8(transfer->getPath());
        #ifdef WIN32
        if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            localPath = localPath.mid(4);
        }
        #endif
        Platform::showInFolder(localPath);
    }
    delete transfer;
}
