#include "MegaTransferDelegate.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include "control/Utilities.h"
#include "gui/QMegaMessageBox.h"
#include "megaapi.h"
#include "QTransfersModel.h"


using namespace mega;

MegaTransferDelegate::MegaTransferDelegate(QTransfersModel *model, QObject *parent)
    : QStyledItemDelegate(parent)
{
    this->model = model;
}

void MegaTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QColor(247, 247, 247));
        }

        int tag = index.internalId();
        TransferItem *ti = model->transferItems[tag];
        if (!ti)
        {
            ti = new TransferItem();
            ti->setTransferTag(tag);
            connect(ti, SIGNAL(refreshTransfer(int)), model, SLOT(refreshTransferItem(int)));
            model->transferItems.insert(tag, ti);
            MegaTransfer *transfer = NULL;

            if (model->getModelType() != QTransfersModel::TYPE_FINISHED)
            {
                transfer = model->megaApi->getTransferByTag(tag);
            }
            else
            {
                transfer = model->getFinishedTransferByTag(tag);
            }

            if (transfer)
            {
                ti->setType(transfer->getType(), transfer->isSyncTransfer());
                ti->setFileName(QString::fromUtf8(transfer->getFileName()));
                ti->setTotalSize(transfer->getTotalBytes());
                ti->setSpeed(transfer->getSpeed());
                ti->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
                ti->setTransferState(transfer->getState());
                ti->setPriority(transfer->getPriority());
                if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
                {
                    ti->setFinishedTime(transfer->getUpdateTime());
                }
            }

            if (model->getModelType() != QTransfersModel::TYPE_FINISHED)
            {
                delete transfer;
            }
        }

        if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
        {
            ti->updateFinishedTime();
        }

        painter->save();
        painter->translate(option.rect.topLeft());
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
        return QSize(800, 48);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

bool MegaTransferDelegate::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (QEvent::MouseButtonRelease ==  event->type())
    {
        int tag = index.internalId();
        TransferItem *item = model->transferItems[tag];
        if (!item)
        {
            return true;
        }

        if (item->cancelButtonClicked(((QMouseEvent *)event)->pos() - option.rect.topLeft()))
        {
            if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
            {
                model->removeTransferByTag(tag);
            }
            else
            {
                QMessageBox warning;
                warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
                warning.setText(tr("Are you sure you want to cancel this transfer?"));
                warning.setIcon(QMessageBox::Warning);
                warning.setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-warning.png")
                                                                                   : QString::fromUtf8(":/images/mbox-warning@2x.png")));
                warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                warning.setDefaultButton(QMessageBox::No);
                int result = warning.exec();
                if (result == QMessageBox::Yes)
                {
                    model->onTransferCancel(tag);
                }
            }
        }
        return true;
    }

    return QAbstractItemDelegate::editorEvent(event, model, option, index);;
}
