#include "MegaTransferDelegate2.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "gui/QMegaMessageBox.h"
#include "megaapi.h"
#include "QTransfersModel2.h"
#include "MegaApplication.h"
#include "platform/Platform.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QToolTip>

using namespace mega;

MegaTransferDelegate2::MegaTransferDelegate2(QTransfersModel2 *model, QWidget* view, QObject *parent)
    : QStyledItemDelegate(parent),
      mModel(model),
      mView(view)
{

}

void MegaTransferDelegate2::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid() && index.data().canConvert<TransferItem2>())
    {
        const auto transferItem (qvariant_cast<TransferItem2>(index.data()));
        const auto nbRowsMaxInView (mView->height()/option.rect.height() + 1);
        const QString widgetName (QLatin1Literal("r")+QString::number(index.row() % nbRowsMaxInView));

        auto w (mView->findChild<TransferManagerItem2 *>(widgetName));

        if (!w)
        {
            w = new TransferManagerItem2(mView);
            w->setObjectName(widgetName);
        }
        w->resize(option.rect.size());
        w->move(option.rect.topLeft());

        w->updateUi(transferItem);

        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, QColor(247, 247, 247));
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
    return QSize(756, 64);
}

void MegaTransferDelegate2::processCancel(int tag)
{
//    if (model->getModelType() == QTransfersModel::TYPE_FINISHED)
//    {
//        model->removeTransferByTag(tag);
//    }
//    else
//    {
//        QPointer<QTransfersModel> modelPointer = model;

//        QMessageBox warning;
//        HighDpiResize hDpiResizer(&warning);
//        warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
//        warning.setText(tr("Are you sure you want to cancel this transfer?"));
//        warning.setIcon(QMessageBox::Warning);
//        warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//        warning.setDefaultButton(QMessageBox::No);
//        int result = warning.exec();
//        if (modelPointer && result == QMessageBox::Yes)
//        {
//            model->megaApi->cancelTransferByTag(tag);
//        }
//    }
}

bool MegaTransferDelegate2::editorEvent(QEvent *event, QAbstractItemModel *, const QStyleOptionViewItem &option, const QModelIndex &index)
{
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

    return QAbstractItemDelegate::editorEvent(event, mModel, option, index);
}

bool MegaTransferDelegate2::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip && index.isValid())
    {
        // Get TransferManagerItem2 widget under cursor
        const QString widgetName (QLatin1Literal("r")+QString::number(index.row()%20));
        auto currentRow (view->findChild<TransferManagerItem2 *>(widgetName));
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

void MegaTransferDelegate2::processShowInFolder(int tag)
{
//    MegaTransfer *transfer = NULL;
//    transfer = model->getTransferByTag(tag);
//    if (transfer && transfer->getState() == MegaTransfer::STATE_COMPLETED
//                 && transfer->getPath())
//    {
//        QString localPath = QString::fromUtf8(transfer->getPath());
//        #ifdef WIN32
//        if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
//        {
//            localPath = localPath.mid(4);
//        }
//        #endif
//        Platform::showInFolder(localPath);
//    }
//    delete transfer;
}

void MegaTransferDelegate2::on_tCancelTransfer_clicked()
{
   // emit transferCanceled(mTransferData.mTag);
}

void MegaTransferDelegate2::on_tPauseTransfer_clicked()
{
   // emit transferPaused(mTransferData.mTag);
}

//void MegaTransferDelegate2::updateUisDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
//{
//    if (topLeft.isValid() && bottomRight.isValid())
//    {
//        for (auto row(topLeft.row()); row <= bottomRight.row(); ++row)
//        {
//            auto ui = mUis[row];

//            // Get new item data
//            auto index (mModel->index(row, 0));
//            const auto transferItem (qvariant_cast<TransferItem2>(index.data()));

//            // Init UI
//            updateUi(ui, transferItem);
//        }
//    }
//}

//void MegaTransferDelegate2::updateUisRowsInserted(const QModelIndex &parent, int first, int last)
//{
//    for (auto row(first); row <= last; ++row)
//    {
//        auto w = new QWidget();
//        auto ui = new Ui::TransferManagerItem();

//        // Get new item data
//        auto index (mModel->index(row, 0, parent));
//        const auto transferItem (qvariant_cast<TransferItem2>(index.data()));
//        // Init UI
//        setupUi(ui, transferItem, w);
//        updateUi(ui, transferItem);

//        QObject::connect(ui->tCancelTransfer, &QToolButton::clicked,
//                         this, &MegaTransferDelegate2::on_tCancelTransfer_clicked);
//        QObject::connect(ui->tPauseTransfer, &QToolButton::clicked,
//                         this, &MegaTransferDelegate2::on_tPauseTransfer_clicked);

//        mUis.insert(row, ui);
//    }
//}

//void MegaTransferDelegate2::updateUisRowsRemoved(const QModelIndex &parent, int first, int last)
//{
//    for (auto row(first); row <= last; ++row)
//    {
//        auto ui = mUis[row];
//        mUis.remove(row);
//        ui->TransferManagerIemLayout->parentWidget()->deleteLater();
//    }
//}

