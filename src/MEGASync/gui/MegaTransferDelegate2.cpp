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
      mItems(new QVector<TransferManagerItem2*>()),
      mView (view)
{
}

TransferManagerItem2* MegaTransferDelegate2::getTransferItemWidget(int row, const QSize& size) const
{
    auto nbRowsMaxInView (mView->height() / size.height() + 1);
    auto item (row % nbRowsMaxInView);

    if (item >= mItems->size())
    {
        auto w = new TransferManagerItem2(mView);
        connect(w, &TransferManagerItem2::cancelClearTransfer,
                this, &MegaTransferDelegate2::onCancelClearTransfer);
        connect(w, &TransferManagerItem2::pauseResumeTransfer,
                this, &MegaTransferDelegate2::onPauseResumeTransfer);
        connect(w, &TransferManagerItem2::retryTransfer,
                mSourceModel, &QTransfersModel2::onRetryTransfer);
        mItems->insert(item, std::move(w));
    }
    return mItems->at(item);
}

void MegaTransferDelegate2::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    auto rowCount (index.model()->rowCount());
    auto row (index.row());

    if (index.isValid() && row < rowCount)
    {
        auto pos (option.rect.topLeft());
        auto width (option.rect.width());
        auto height (option.rect.height());
        auto transferItem (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
        TransferManagerItem2* w (getTransferItemWidget(row, option.rect.size()));

        // Move if position changed
        if (w->pos() != pos)
        {
            w->move(pos);
        }

        // Resize if window resized
        if (w->width() != width)
        {
            w->resize(width, height);
        }

        // Update data
        w->updateUi(transferItem.getTransferData(), row);

        // Draw border if selected
        if (option.state & QStyle::State_Selected)
        {
            static const QPen pen (QColor::fromRgbF(0.84, 0.84, 0.84, 1), 1);
            QPainterPath path;
            path.addRoundedRect(QRectF(option.rect.x() + 16.,
                                       option.rect.y() + 4.,
                                       width - 17.,
                                       height - 7.),
                                10, 10);
            painter->setPen(pen);
            painter->fillPath(path, Qt::white);
            painter->drawPath(path);
        }
        painter->save();

        painter->translate(pos);
        w->render(painter, QPoint(0, 0), QRegion(0, 0, width, height));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaTransferDelegate2::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    return QSize(772, 64);
}

void MegaTransferDelegate2::onCancelClearTransfer(int row)
{
    QModelIndexList indexes;
    auto proxy(qobject_cast<QSortFilterProxyModel*>(mModel));
    auto index (mModel->index(row, 0, QModelIndex()));
    if (proxy)
    {
        index = proxy->mapToSource(index);
    }
    indexes.push_back(index);
    mSourceModel->cancelClearTransfers(indexes);
}

void MegaTransferDelegate2::onPauseResumeTransfer(int row, bool pauseState)
{
    QModelIndexList indexes;
    auto proxy(qobject_cast<QSortFilterProxyModel*>(mModel));
    auto index (mModel->index(row, 0, QModelIndex()));
    if (proxy)
    {
        index = proxy->mapToSource(index);
    }
    indexes.push_back(index);
    mSourceModel->pauseTransfers(indexes, pauseState);
}

bool MegaTransferDelegate2::editorEvent(QEvent* event, QAbstractItemModel* model,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index)
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
                    auto currentRow (getTransferItemWidget(index.row(), option.rect.size()));
                    currentRow->forwardMouseEvent(me);
                }
                break;
            }
            default:
                break;
        }
    }
    return QStyledItemDelegate::editorEvent(event, mModel, option, index);
}

bool MegaTransferDelegate2::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                      const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::ToolTip && index.isValid())
    {
        auto currentRow (getTransferItemWidget(index.row(), option.rect.size()));
        auto widget (currentRow->childAt(event->pos() - currentRow->pos()));
        if (widget)
        {
            QToolTip::showText(event->globalPos(), widget->toolTip());
        }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}
