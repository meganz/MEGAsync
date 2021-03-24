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
        connect(w, &TransferManagerItem2::cancelClearTransfer,
                this, &MegaTransferDelegate2::onCancelClearTransfer);
        connect(w, &TransferManagerItem2::pauseResumeTransfer,
                this, &MegaTransferDelegate2::onPauseResumeTransfer);
        connect(w, &TransferManagerItem2::retryTransfer,
                mSourceModel, &QTransfersModel2::onRetryTransfer);
    }
    return w;
}

void MegaTransferDelegate2::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const
{
    if (index.isValid())
    {
        auto transferItem (qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
        auto w (getTransferItemWidget(index.row(), option.rect.height()));
        w->resize(option.rect.size());
        w->move(option.rect.topLeft());
        w->updateUi(transferItem.getTransferData(), index.row());

        if (option.state & QStyle::State_Selected)
        {
            static const QPen pen (QColor::fromRgbF(0.84, 0.84, 0.84, 1), 1);
            QPainterPath path;
            path.addRoundedRect(QRectF(option.rect.x() + 16.,
                                       option.rect.y() + 4.,
                                       option.rect.width() - 17.,
                                       option.rect.height() - 7.),
                                10, 10);
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

QSize MegaTransferDelegate2::sizeHint(const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    return QSize(720, 64);
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
                    auto currentRow (getTransferItemWidget(index.row(), option.rect.height()));
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
