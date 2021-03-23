#include "MegaTransferDelegate2.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "gui/QMegaMessageBox.h"
#include "megaapi.h"
#include "QTransfersModel2.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "TransfersWidget.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QToolTip>
#include <QSortFilterProxyModel>

using namespace mega;

MegaTransferDelegate2::MegaTransferDelegate2(QAbstractItemModel* model, QWidget* view,
                                             QObject *parent)
    : QStyledItemDelegate(parent),
      mModel (model),
      mSourceModel (qobject_cast<QTransfersModel2*>(
                        qobject_cast<TransfersSortFilterProxyModel*>(mModel)->sourceModel())),
      mView (view)
{
}

TransferManagerItem2* MegaTransferDelegate2::getTransferItemWidget(int row, int itemHeight) const
{
    const auto nbRowsMaxInView (mView->height() / itemHeight + 1);
    const QString widgetName (QLatin1Literal("r") + QString::number(row % nbRowsMaxInView));

    auto w (mView->findChild<TransferManagerItem2*>(widgetName));

    if (!w)
    {
        w = new TransferManagerItem2(mView);
        w->setObjectName(widgetName);
        connect(w, &TransferManagerItem2::cancelClearTransfers,
                this, &MegaTransferDelegate2::onCancelClearTransfers);
        connect(w, &TransferManagerItem2::retryTransfer,
                mSourceModel, &QTransfersModel2::onRetryTransfer);
    }
    return w;
}

void MegaTransferDelegate2::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex &index) const
{
    if (index.isValid() && (index.data(Qt::DisplayRole).canConvert<TransferItem2>()))
    {
        auto transferItem (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
        auto w (getTransferItemWidget(index.row(), option.rect.height()));
        w->resize(option.rect.size());
        w->move(option.rect.topLeft());
        w->updateUi(transferItem.getTransferData(), index.row());

        if (option.state & QStyle::State_Selected)
        {
            QPainterPath path;
            path.addRoundedRect(QRectF(option.rect.x() + 16.,
                                       option.rect.y() + 4.,
                                       option.rect.width() - 17.,
                                       option.rect.height() - 7.),
                                10, 10);
            QPen pen (QColor::fromRgbF(0.84, 0.84, 0.84, 1), 1);
            painter->setPen(pen);
            painter->fillPath(path, Qt::white);
            painter->drawPath(path);
        }
        painter->save();

        painter->translate(option.rect.topLeft());
        w->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaTransferDelegate2::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(720, 64);
}

void MegaTransferDelegate2::onCancelClearTransfers(int firstRow, int count)
{
    QModelIndexList indexes;
    auto proxy(qobject_cast<QSortFilterProxyModel*>(mModel));
    auto index (mModel->index(firstRow, 0, QModelIndex()));
    if (proxy)
    {
        index = proxy->mapToSource(index);
    }
    indexes.push_back(index);
    mSourceModel->cancelClearTransfers(indexes);
}

bool MegaTransferDelegate2::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.isValid())
    {
        switch (event->type())
        {
            case QEvent::MouseButtonPress:
            {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                if( me->button() == Qt::LeftButton )
                {
                    auto currentRow (getTransferItemWidget(index.row(), option.rect.height()));
                    currentRow->forwardMouseEvent(me);
                }
                break;
            }
            default:
                break;
        }
    }


    //    if (QEvent::MouseButtonPress ==  event->type())
//    {
//        int tag = index.internalId();
//        TransferItem *item = model->transferItems[tag];
//        if (!item)
//        {
//            return true;
//        }

//        if (item->cancelButtonClicked(((QMouseEvent *)event)->pos() - option.rect.topLeft()))
//        {
//            processCancel(tag);
//            return true; // click consumed
//        }
//        else if (item && item->checkIsInsideButton(((QMouseEvent *)event)->pos() - option.rect.topLeft(), TransferItem::ACTION_BUTTON))
//        {
//            int modelType = model->getModelType();
//            if (modelType == QTransfersModel::TYPE_FINISHED
//                    || modelType == QTransfersModel::TYPE_CUSTOM_TRANSFERS)
//            {
//                const auto &ti = model->transferItems[tag];
//                if (modelType == QTransfersModel::TYPE_CUSTOM_TRANSFERS && !ti->getIsLinkAvailable() && !ti->getTransferError())
//                {
//                    processShowInFolder(tag);
//                }
//                else if (MegaTransfer *transfer = model->getTransferByTag(tag) )
//                {
//                    if (!transfer->getLastError().getErrorCode())
//                    {
//                        QList<MegaHandle> exportList;
//                        QStringList linkList;

//                        MegaNode *node = transfer->getPublicMegaNode();
//                        if (!node || !node->isPublic())
//                        {
//                            exportList.push_back(transfer->getNodeHandle());
//                        }
//                        else
//                        {
//                            char *handle = node->getBase64Handle();
//                            char *key = node->getBase64Key();
//                            if (handle && key)
//                            {
//                                QString link = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2")
//                                        .arg(QString::fromUtf8(handle)).arg(QString::fromUtf8(key));
//                                linkList.append(link);
//                            }
//                            delete [] handle;
//                            delete [] key;
//                        }
//                        delete node;

//                        if (exportList.size() || linkList.size())
//                        {
//                            ((MegaApplication*)qApp)->exportNodes(exportList, linkList);
//                        }
//                    }
//                    else
//                    {
//                        ((MegaApplication*)qApp)->getMegaApi()->retryTransfer(transfer);
//                    }

//                    delete transfer;
//                }
//             }
//             return true; // click consumed
//        }
//        else if (item && item->checkIsInsideButton(((QMouseEvent *)event)->pos() - option.rect.topLeft(), TransferItem::SHOW_IN_FOLDER_BUTTON))
//        {
//            if (model->getModelType() == QTransfersModel::TYPE_CUSTOM_TRANSFERS)
//            {
//                processShowInFolder(tag);
//                return true; // click consumed
//            }
//            // we are not consuming the click; fall through to do the usual thing (of selecting the clicked row)
//        }
//    }
//    else if (QEvent::MouseButtonDblClick == event->type() && model->getModelType() == QTransfersModel::TYPE_FINISHED)
//    {
//        int tag = index.internalId();
//        processShowInFolder(tag);
//        return true; // double-click consumed
//    }

    return QStyledItemDelegate::editorEvent(event, mModel, option, index);
}

bool MegaTransferDelegate2::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip && index.isValid())
    {
        // Get TransferManagerItem2 widget under cursor
        const auto nbRowsMaxInView (mView->height()/option.rect.height() + 1);
        const QString widgetName (QLatin1Literal("r")+QString::number(index.row() % nbRowsMaxInView));
        auto currentRow (view->findChild<TransferManagerItem2*>(widgetName));
        if (currentRow)
        {
            // Get widget inside TransferManagerItem2 under cursor, and display its tooltip
            auto widget (currentRow->childAt(event->pos() - currentRow->pos()));
            if (widget)
            {
                QToolTip::showText(event->globalPos(), widget->toolTip());
            }
        }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}



