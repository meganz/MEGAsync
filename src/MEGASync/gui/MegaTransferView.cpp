#include "MegaTransferView.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "control/Utilities.h"
#include "gui/QMegaMessageBox.h"
#include "TransfersWidget.h"

#include <QScrollBar>
#include <QtConcurrent/QtConcurrent>
#include <QIcon>

using namespace mega;

MegaTransferView::MegaTransferView(QWidget *parent) :
    QTreeView(parent),
    mParentTransferWidget(nullptr),
    mContextMenu(nullptr),
    mPauseAction(nullptr),
    mResumeAction(nullptr),
    mMoveToTopAction(nullptr),
    mMoveUpAction(nullptr),
    mMoveDownAction(nullptr),
    mMoveToBottomAction(nullptr),
    mCancelAction(nullptr),
    mGetLinkAction(nullptr),
    mOpenItemAction(nullptr),
    mShowInFolderAction(nullptr),
    mShowInMegaAction(nullptr),
    mClearAction(nullptr)
{
    setMouseTracking(true);
    lastItemHoveredTag = 0;
    disableLink = false;
    disableMenus = false;
    type = 0;
}

void MegaTransferView::setup(int type)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    // Disable and find out alternative way to position context menu,
    // since main parent widget is flagged as popup (InfoDialog), and coordinates does not work properly
    // connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    //createContextMenu();
//    createCompletedContextMenu();
   // connect(this, &MegaTransferView::showContextMenu, this, &MegaTransferView::onCustomContextMenu);

    this->type = type;
}

void MegaTransferView::setup(TransfersWidget *tw)
{
    mParentTransferWidget = tw;
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Disable and find out alternative way to position context menu,
    // since main parent widget is flagged as popup (InfoDialog), and coordinates does not work properly
    connect(this, &MegaTransferView::showContextMenu,
            this, &MegaTransferView::onCustomContextMenu);
    createContextMenu();
}

void MegaTransferView::disableGetLink(bool disable)
{
    disableLink = disable;
    mGetLinkAction->setEnabled(!disable);
}

int MegaTransferView::getType() const
{
    return type;
}

void MegaTransferView::pauseResumeSelection(bool pauseState)
{
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));

    auto selection = selectionModel()->selection();
    QModelIndexList indexes;

    if (selection.size() > 0)
    {
        if (proxy)
        {
            selection = proxy->mapSelectionToSource(selection);
        }
        indexes = selection.indexes();

        clearSelection();
    }
    else
    {
        auto rowCount (model()->rowCount());
        for (auto row (0); row < rowCount; ++row)
        {
            auto index (model()->index(row, 0, QModelIndex()));
            if (proxy)
            {
                index = proxy->mapToSource(index);
            }
            indexes.push_back(index);
        }
    }
    mParentTransferWidget->getModel2()->pauseTransfers(indexes, pauseState);
}

void MegaTransferView::cancelClearSelection()
{
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));

    auto selection = selectionModel()->selection();
    QModelIndexList indexes;

    if (selection.size() > 0)
    {
        if (proxy)
        {
            selection = proxy->mapSelectionToSource(selection);
        }
        indexes = selection.indexes();

        clearSelection();
    }
    else
    {
        auto rowCount (model()->rowCount());
        for (auto row (0); row < rowCount; ++row)
        {
            auto index (model()->index(row, 0, QModelIndex()));
            if (proxy)
            {
                index = proxy->mapToSource(index);
            }
            indexes.push_back(index);
        }
    }
    mParentTransferWidget->getModel2()->cancelClearTransfers(indexes);
}


void MegaTransferView::disableContextMenus(bool option)
{
    disableMenus = option;
}

void MegaTransferView::createContextMenu()
{
    if (!mContextMenu)
    {
        mContextMenu = new QMenu(this);
    }
    else
    {        
        for (auto action : mContextMenu->actions())
        {
            mContextMenu->removeAction(action);
        }
    }

    if (mPauseAction)
    {
        mPauseAction->deleteLater();
        mPauseAction = nullptr;
    }

    mPauseAction = new QAction(QIcon(QLatin1String(":/images/pause_ico.png")),
                                     tr("Pause active transfers"), this);
    connect(mPauseAction, &QAction::triggered,
            mParentTransferWidget, &TransfersWidget::on_tPauseResumeAll_clicked);

    if (mResumeAction)
    {
        mResumeAction->deleteLater();
        mResumeAction = nullptr;
    }

    mResumeAction = new QAction(QIcon(QLatin1String(":/images/play_ico.png")),
                                      tr("Resume paused transfers"), this);
    connect(mResumeAction, &QAction::triggered,
            mParentTransferWidget, &TransfersWidget::on_tPauseResumeAll_clicked);

    if (mMoveToTopAction)
    {
        mMoveToTopAction->deleteLater();
        mMoveToTopAction = nullptr;
    }

    mMoveToTopAction = new QAction(QIcon(QLatin1String(":/images/ico_move_to_top.png")),
                                         tr("Move to top"), this);
    connect(mMoveToTopAction, &QAction::triggered, this, &MegaTransferView::moveToTopClicked);

    if (mMoveUpAction)
    {
        mMoveUpAction->deleteLater();
        mMoveUpAction = nullptr;
    }

    mMoveUpAction = new QAction(QIcon(QLatin1String(":/images/ico_move_up.png")),
                                      tr("Move up"), this);
    connect(mMoveUpAction, &QAction::triggered, this, &MegaTransferView::moveUpClicked);

    if (mMoveDownAction)
    {
        mMoveDownAction->deleteLater();
        mMoveDownAction = nullptr;
    }

    mMoveDownAction = new QAction(QIcon(QLatin1String(":/images/ico_move_down.png")),
                                        tr("Move down"), this);
    connect(mMoveDownAction, &QAction::triggered, this, &MegaTransferView::moveDownClicked);


    if (mMoveToBottomAction)
    {
        mMoveToBottomAction->deleteLater();
        mMoveToBottomAction = nullptr;
    }

    mMoveToBottomAction = new QAction(QIcon(QLatin1String(":/images/ico_move_to_bottom.png")),
                                            tr("Move to bottom"), this);
    connect(mMoveToBottomAction, &QAction::triggered, this, &MegaTransferView::moveToBottomClicked);

    if (mCancelAction)
    {
        mCancelAction->deleteLater();
        mCancelAction = nullptr;
    }

    mCancelAction = new QAction(QIcon(QLatin1String(":/images/clear_item_ico.png")),
                                      tr("Cancel transfers in progress"), this);
    connect(mCancelAction, &QAction::triggered,
            mParentTransferWidget, &TransfersWidget::on_tCancelAll_clicked);

    if (mGetLinkAction)
    {
        mGetLinkAction->deleteLater();
        mGetLinkAction = nullptr;
    }

    mGetLinkAction = new QAction(QIcon(QLatin1String(":/images/get_link_ico.png")),
                                 tr("Get link"), this);
    connect(mGetLinkAction, &QAction::triggered, this, &MegaTransferView::getLinkClicked);

    if (mOpenItemAction)
    {
        mOpenItemAction->deleteLater();
        mOpenItemAction = nullptr;
    }

    mOpenItemAction = new QAction(QIcon(QLatin1String(":/images/ico_open.png")),
                                  tr("Open"), this);
    connect(mOpenItemAction, &QAction::triggered, this, &MegaTransferView::openItemClicked);

    if (mShowInFolderAction)
    {
        mShowInFolderAction->deleteLater();
        mShowInFolderAction = nullptr;
    }

    mShowInFolderAction = new QAction(QIcon(QLatin1String(":/images/ico_show_in_folder.png")),
                                      tr("Show in folder"), this);
    connect(mShowInFolderAction, &QAction::triggered, this, &MegaTransferView::showInFolderClicked);

    if (mShowInMegaAction)
    {
        mShowInMegaAction->deleteLater();
        mShowInMegaAction = nullptr;
    }

    mShowInMegaAction = new QAction(QIcon(QLatin1String(":/images/ico_about_MEGA.png")),
                                    tr("Show in Mega"), this);
    connect(mShowInMegaAction, &QAction::triggered, this, &MegaTransferView::showInMegaClicked);

    if (mClearAction)
    {
        mClearAction->deleteLater();
        mClearAction = nullptr;
    }

    mClearAction = new QAction(QIcon(QLatin1String(":/images/ico_clear.png")),
                               tr("Clear completed"), this);
    connect(mClearAction, &QAction::triggered,
            mParentTransferWidget, &TransfersWidget::on_tCancelAll_clicked);

    mContextMenu->addAction(mPauseAction);
    mContextMenu->addAction(mResumeAction);

    mContextMenu->addAction(mOpenItemAction);
    mContextMenu->addAction(mShowInFolderAction);
    mContextMenu->addAction(mShowInMegaAction);

    mContextMenu->addSeparator();

    mContextMenu->addAction(mGetLinkAction);

    mContextMenu->addAction(mMoveToTopAction);
    mContextMenu->addAction(mMoveUpAction);
    mContextMenu->addAction(mMoveDownAction);
    mContextMenu->addAction(mMoveToBottomAction);

    mContextMenu->addSeparator();

    mContextMenu->addAction(mCancelAction);
    mContextMenu->addAction(mClearAction);
}

void MegaTransferView::updateContextMenu(bool enablePause, bool enableResume, bool enableMove, bool enableClear, bool enableCancel)
{
    mPauseAction->setVisible(enablePause);
    mResumeAction->setVisible(enableResume);
    mMoveToTopAction->setVisible(enableMove);
    mMoveUpAction->setVisible(enableMove);
    mMoveToBottomAction->setVisible(enableMove);
    mMoveDownAction->setVisible(enableMove);
    mCancelAction->setVisible(enableCancel);
    mClearAction->setVisible(enableClear);

    bool onlyOneSelected (enableClear && (selectedIndexes().size() == 1));
    mGetLinkAction->setVisible(onlyOneSelected);
    mOpenItemAction->setVisible(onlyOneSelected);
    mShowInFolderAction->setVisible(onlyOneSelected);
    mShowInMegaAction->setVisible(onlyOneSelected);
}

void MegaTransferView::mouseMoveEvent(QMouseEvent *event)
{
    auto model = this->model();
//    if (model)
//    {
//        QModelIndex index = indexAt(event->pos());
//        if (index.isValid())
//        {
//            int tag = index.internalId();
//            if (lastItemHoveredTag)
//            {
//                TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
//                if (lastItemHovered)
//                {
//                    lastItemHovered->mouseHoverTransfer(false, event->pos() - visualRect(index).topLeft());
//                }
//            }

//            TransferItem *item = model->transferItems[tag];
//            if (item)
//            {
//                lastItemHoveredTag = item->getTransferTag();
//                item->mouseHoverTransfer(true, event->pos() - visualRect(index).topLeft());
//            }
//            else
//            {
//                lastItemHoveredTag = 0;
//            }
//        }
//        else
//        {
//            if (lastItemHoveredTag)
//            {
//                TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
//                if (lastItemHovered)
//                {
//                    lastItemHovered->mouseHoverTransfer(false, event->pos() - visualRect(index).topLeft());
//                    update();
//                }
//                lastItemHoveredTag = 0;
//            }
//        }
//    }
    QTreeView::mouseMoveEvent(event);
}

void MegaTransferView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!(event->button() == Qt::RightButton))
    {
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    if (!disableMenus)
    {
        emit showContextMenu(QPoint(event->x(), event->y()));
    }
    QTreeView::mouseReleaseEvent(event);
}

void MegaTransferView::leaveEvent(QEvent *event)
{
//    QTransfersModel *model = (QTransfersModel*)this->model();
//    if (model)
//    {
//        if (lastItemHoveredTag)
//        {
//            TransferItem *lastItemHovered = model->transferItems[lastItemHoveredTag];
//            if (lastItemHovered)
//            {
//                lastItemHovered->mouseHoverTransfer(false, QPoint(-1,-1));
//                update();
//            }
//            lastItemHoveredTag = 0;
//        }
//    }
    QTreeView::leaveEvent(event);
}

void MegaTransferView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        createContextMenu();
    }
    QWidget::changeEvent(event);
}

void MegaTransferView::dropEvent(QDropEvent *event)
{
    QAbstractItemView::dropEvent(event);
    clearSelection();
}

//void MegaTransferView::paintEvent(QPaintEvent * e)
//{
//    auto app = static_cast<MegaApplication*>(qApp);
//    app->megaApiLock.reset(app->getMegaApi()->getMegaApiLock(false));
//    QTreeView::paintEvent(e);
//    app->megaApiLock.reset();
//}

void MegaTransferView::onCustomContextMenu(const QPoint &point)
{
    bool enablePause = false;
    bool enableResume = false;
    bool enableCancel = false;
    bool enableClear = false;
    bool enableMove = false;

    QModelIndexList indexes = selectedIndexes();

    for (auto index : qAsConst(indexes))
    {
        auto d (qvariant_cast<TransferItem2>(index.data()).getTransferData());

        switch (d->mState)
        {
            case MegaTransfer::STATE_ACTIVE:
            case MegaTransfer::STATE_QUEUED:
            case MegaTransfer::STATE_RETRYING:
            {
                enablePause = true;
                enableMove = true;
                enableCancel = true;
                break;
            }
            case MegaTransfer::STATE_PAUSED:
            {
                enableResume = true;
                enableMove = true;
                enableCancel = true;
                break;
            }
            case MegaTransfer::STATE_CANCELLED:
            case MegaTransfer::STATE_FAILED:
            case MegaTransfer::STATE_COMPLETED:
            {
                enableClear = true;
                break;
            }
            default:
                break;
        }
    }
    updateContextMenu(enablePause, enableResume, enableMove,
                      enableClear, enableCancel);
    mContextMenu->exec(mapToGlobal(point));
}

void MegaTransferView::moveToTopClicked()
{

    auto selection = selectionModel()->selection();
    auto m (model());
    auto proxy (qobject_cast<QSortFilterProxyModel*>(m));
    if (proxy)
    {
        selection = proxy->mapSelectionToSource(selection);
        m = proxy->sourceModel();
    }

    auto indexes = selection.indexes();

    // Reverse sort to keep items in the same order
    std::sort(indexes.rbegin(), indexes.rend());

    for (auto index : qAsConst(indexes))
    {
        m->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), 0);
    }
    clearSelection();
}

void MegaTransferView::moveUpClicked()
{
    auto indexes = selectionModel()->selectedRows();
    // Sort to keep items in the same order
    std::sort(indexes.begin(), indexes.end());

    for (auto index : qAsConst(indexes))
    {
        int row(index.row());
        model()->moveRows(QModelIndex(), row, 1, QModelIndex(), row - 1);
    }
    clearSelection();
}

void MegaTransferView::moveDownClicked()
{
    auto indexes = selectionModel()->selectedRows();
    // Reverse sort to keep items in the same order
    std::sort(indexes.rbegin(), indexes.rend());

    for (auto index : qAsConst(indexes))
    {
            int row(index.row());
            model()->moveRows(QModelIndex(), row, 1, QModelIndex(),
                              std::min(row + 2, model()->rowCount()));
    }
    clearSelection();
}

void MegaTransferView::moveToBottomClicked()
{
    auto selection = selectionModel()->selection();
    auto m (model());
    auto proxy (qobject_cast<QSortFilterProxyModel*>(m));
    if (proxy)
    {
        selection = proxy->mapSelectionToSource(selection);
        m = proxy->sourceModel();
    }

    auto indexes = selection.indexes();
    // Sort to keep items in the same order
    std::sort(indexes.begin(), indexes.end());

    for (auto index : qAsConst(indexes))
    {
            m->moveRows(QModelIndex(), index.row(), 1, QModelIndex(), m->rowCount());
    }
    clearSelection();
}

void MegaTransferView::getLinkClicked()
{
    if (disableLink)
    {
        return;
    }

    QList<int> rows;
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));

    const auto  selection (proxy ? proxy->mapSelectionToSource(selectionModel()->selection())
                                 : selectionModel()->selection());


    for (auto index : selection.indexes())
    {
        if (index.isValid())
        {
            rows.push_back(index.row());
        }
    }

    if (!rows.isEmpty())
    {
        mParentTransferWidget->getModel2()->getLinks(rows);
    }

    clearSelection();
}

void MegaTransferView::openItemClicked()
{
    const QModelIndexList selection (selectedIndexes());
    for (auto index : selection)
    {
        if (index.isValid())
        {
            const auto transferItem (
                        qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
            auto d (transferItem.getTransferData());
            if (!d->mPath.isEmpty())
            {
                QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(d->mPath));
            }
        }
    }
    clearSelection();
}

void MegaTransferView::showInFolderClicked()
{
    const QModelIndexList selection (selectedIndexes());
    for (auto index : selection)
    {
        if (index.isValid())
        {
            const auto transferItem (
                        qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
            auto d (transferItem.getTransferData());
            if (!d->mPath.isEmpty())
            {
                Platform::showInFolder(d->mPath);
            }
        }
    }
    clearSelection();
}

void MegaTransferView::showInMegaClicked()
{
    const QModelIndexList selection (selectedIndexes());
    for (auto index : selection)
    {
        if (index.isValid())
        {
            const auto transferItem (
                        qvariant_cast<TransferItem2>(index.data(Qt::DisplayRole)));
            auto d (transferItem.getTransferData());
            if (d->mParentHandle != mega::INVALID_HANDLE)
            {
                qobject_cast<MegaApplication*>(qApp)->shellViewOnMega(d->mParentHandle, false);
            }
        }
    }
    clearSelection();
}
