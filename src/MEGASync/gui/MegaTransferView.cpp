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

MegaTransferView::MegaTransferView(QWidget* parent) :
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
    this->type = type;
}

void MegaTransferView::setup(TransfersWidget* tw)
{
    mParentTransferWidget = tw;
    setContextMenuPolicy(Qt::CustomContextMenu);

    // Disable and find out alternative way to position context menu,
    // since main parent widget is flagged as popup (InfoDialog), and coordinates does not work properly
    connect(this, &MegaTransferView::showContextMenu,
            this, &MegaTransferView::onCustomContextMenu);

    connect(tw, &TransfersWidget::pauseResumeAllRows,
            this, &MegaTransferView::onPauseResumeAllRows, Qt::QueuedConnection);

    connect(tw, &TransfersWidget::cancelClearAllRows,
            this, &MegaTransferView::onCancelClearAllRows, Qt::QueuedConnection);

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

void MegaTransferView::onPauseResumeAllRows(bool pauseState)
{
    QModelIndexList indexes;
    auto rowCount (model()->rowCount());
    if (rowCount > 0)
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        for (auto row (0); row < rowCount; ++row)
        {
            auto index (model()->index(row, 0, QModelIndex()));
            if (proxy)
            {
                index = proxy->mapToSource(index);
            }
            indexes.push_back(index);
        }
        mParentTransferWidget->getModel2()->pauseTransfers(indexes, pauseState);
    }
}

void MegaTransferView::onPauseResumeSelection(bool pauseState)
{
    auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
    auto selection = selectionModel()->selection();

    if (selection.size() > 0)
    {
        QModelIndexList indexes;

        if (proxy)
        {
            selection = proxy->mapSelectionToSource(selection);
        }
        indexes = selection.indexes();

        clearSelection();
        mParentTransferWidget->getModel2()->pauseTransfers(indexes, pauseState);
    }
}

void MegaTransferView::onCancelClearAllRows(bool cancel, bool clear)
{
    QModelIndexList indexes;
    auto rowCount (model()->rowCount());
    if (rowCount > 0)
    {
        auto proxy(qobject_cast<QSortFilterProxyModel*>(model()));
        for (auto row (0); row < rowCount; ++row)
        {
            auto index (model()->index(row, 0, QModelIndex()));
            if (proxy)
            {
                index = proxy->mapToSource(index);
            }
            indexes.push_back(index);
        }
        QtConcurrent::run([=]
        {
            mParentTransferWidget->getModel2()->cancelClearTransfers(indexes, cancel, clear);
        });
    }
}

void MegaTransferView::onCancelClearSelection(bool cancel, bool clear)
{
    auto proxy (qobject_cast<QSortFilterProxyModel*>(model()));

    auto selection (selectionModel()->selection());

    if (selection.size() > 0)
    {
        QModelIndexList indexes;

        if (proxy)
        {
            indexes = proxy->mapSelectionToSource(selection).indexes();
        }
        else
        {
            indexes = selection.indexes();
        }

        clearSelection();
        mParentTransferWidget->getModel2()->cancelClearTransfers(indexes, cancel, clear);
    }
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
                                     tr("Pause Transfer(s)"), this);
    connect(mPauseAction, &QAction::triggered,
            this, &MegaTransferView::pauseSelectedClicked);

    if (mResumeAction)
    {
        mResumeAction->deleteLater();
        mResumeAction = nullptr;
    }

    mResumeAction = new QAction(QIcon(QLatin1String(":/images/resume_ico.png")),
                                      tr("Resume Transfer(s)"), this);
    connect(mResumeAction, &QAction::triggered,
            this, &MegaTransferView::resumeSelectedClicked);

    if (mMoveToTopAction)
    {
        mMoveToTopAction->deleteLater();
        mMoveToTopAction = nullptr;
    }

    mMoveToTopAction = new QAction(QIcon(QLatin1String(":/images/move_top_ico.png")),
                                         tr("Move to top"), this);
    connect(mMoveToTopAction, &QAction::triggered, this, &MegaTransferView::moveToTopClicked);

    if (mMoveUpAction)
    {
        mMoveUpAction->deleteLater();
        mMoveUpAction = nullptr;
    }

    mMoveUpAction = new QAction(QIcon(QLatin1String(":/images/move_up_ico.png")),
                                      tr("Move up"), this);
    connect(mMoveUpAction, &QAction::triggered, this, &MegaTransferView::moveUpClicked);

    if (mMoveDownAction)
    {
        mMoveDownAction->deleteLater();
        mMoveDownAction = nullptr;
    }

    mMoveDownAction = new QAction(QIcon(QLatin1String(":/images/move_down_ico.png")),
                                        tr("Move down"), this);
    connect(mMoveDownAction, &QAction::triggered, this, &MegaTransferView::moveDownClicked);

    if (mMoveToBottomAction)
    {
        mMoveToBottomAction->deleteLater();
        mMoveToBottomAction = nullptr;
    }

    mMoveToBottomAction = new QAction(QIcon(QLatin1String(":/images/move_bottom_ico.png")),
                                            tr("Move to bottom"), this);
    connect(mMoveToBottomAction, &QAction::triggered, this, &MegaTransferView::moveToBottomClicked);

    if (mCancelAction)
    {
        mCancelAction->deleteLater();
        mCancelAction = nullptr;
    }

    mCancelAction = new QAction(QIcon(QLatin1String(":/images/cancel_transfer_ico.png")),
                                      tr("Cancel Transfer(s)"), this);
    connect(mCancelAction, &QAction::triggered,
            this, &MegaTransferView::cancelSelectedClicked);

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

    mOpenItemAction = new QAction(QIcon(QLatin1String(":/images/open_file_ico.png")),
                                  tr("Open"), this);
    connect(mOpenItemAction, &QAction::triggered, this, &MegaTransferView::openItemClicked);

    if (mShowInFolderAction)
    {
        mShowInFolderAction->deleteLater();
        mShowInFolderAction = nullptr;
    }

    mShowInFolderAction = new QAction(QIcon(QLatin1String(":/images/show_in_folder_ico.png")),
                                      tr("Show in folder"), this);
    connect(mShowInFolderAction, &QAction::triggered, this, &MegaTransferView::showInFolderClicked);

    if (mClearAction)
    {
        mClearAction->deleteLater();
        mClearAction = nullptr;
    }

    mClearAction = new QAction(QIcon(QLatin1String(":/images/ico_clear.png")),
                               tr("Clear"), this);
    connect(mClearAction, &QAction::triggered,
            this, &MegaTransferView::clearSelectedClicked);

    mContextMenu->addAction(mPauseAction);
    mContextMenu->addAction(mResumeAction);

    mContextMenu->addAction(mOpenItemAction);
    mContextMenu->addAction(mShowInFolderAction);

    mContextMenu->addSeparator();

    mContextMenu->addAction(mGetLinkAction);

    mContextMenu->addAction(mMoveToTopAction);
    mContextMenu->addAction(mMoveUpAction);
    mContextMenu->addAction(mMoveDownAction);
    mContextMenu->addAction(mMoveToBottomAction);

    mContextMenu->addSeparator();

    mContextMenu->addAction(mCancelAction);
    mContextMenu->addAction(mClearAction);

    // Set default action to have it painted red
    mContextMenu->setDefaultAction(mCancelAction);
}

void MegaTransferView::updateContextMenu(bool enablePause, bool enableResume, bool enableMove,
                                         bool enableClear, bool enableCancel)
{
    mPauseAction->setVisible(enablePause);
    mResumeAction->setVisible(enableResume);
    mMoveToTopAction->setVisible(enableMove);
    mMoveUpAction->setVisible(enableMove);
    mMoveToBottomAction->setVisible(enableMove);
    mMoveDownAction->setVisible(enableMove);
    mCancelAction->setVisible(enableCancel);
    mClearAction->setVisible(enableClear);

    bool onlyOneSelected ((selectedIndexes().size() == 1));
    bool onlyOneAndClear(enableClear && onlyOneSelected);

    bool showLink (false);
    bool showOpen (false);
    bool showShowInFolder (false);

    if (onlyOneAndClear)
    {
        auto d (qvariant_cast<TransferItem2>(selectedIndexes().first().data()).getTransferData());

        auto state (d->mState);
        auto type ((d->mType & TransferData::TRANSFER_UPLOAD) ?
                       TransferData::TRANSFER_UPLOAD
                     : TransferData::TRANSFER_DOWNLOAD);

        if (state == TransferData::TRANSFER_COMPLETED)
        {
            showLink = true;
            showOpen = true;
            showShowInFolder = true;
        }
        else if (type == TransferData::TRANSFER_UPLOAD)
        {
            showOpen = true;
            showShowInFolder = true;
        }
        else if (type == TransferData::TRANSFER_DOWNLOAD)
        {
            showLink = true;
        }
    }

    mGetLinkAction->setVisible(showLink);
    mOpenItemAction->setVisible(showOpen);
    mShowInFolderAction->setVisible(showShowInFolder);

    if (onlyOneSelected)
    {
        mPauseAction->setText(tr("Pause Transfer"));
        mResumeAction->setText(tr("Resume Transfer"));
        mCancelAction->setText(tr("Cancel Transfer"));
    }
    else
    {
        mPauseAction->setText(tr("Pause Transfers"));
        mResumeAction->setText(tr("Resume Transfers"));
        mCancelAction->setText(tr("Cancel Transfers"));
    }
}

void MegaTransferView::mouseReleaseEvent(QMouseEvent* event)
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

void MegaTransferView::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        createContextMenu();
    }
    QTreeView::changeEvent(event);
}

void MegaTransferView::dropEvent(QDropEvent* event)
{
    QAbstractItemView::dropEvent(event);
    clearSelection();
}

void MegaTransferView::onCustomContextMenu(const QPoint& point)
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
            case TransferData::TRANSFER_ACTIVE:
            case TransferData::TRANSFER_QUEUED:
            case TransferData::TRANSFER_RETRYING:
            {
                enablePause = true;
                enableMove = true;
                enableCancel = !(d->mType & TransferData::TRANSFER_SYNC);
                break;
            }
            case TransferData::TRANSFER_PAUSED:
            {
                enableResume = true;
                enableMove = true;
                enableCancel = !(d->mType & TransferData::TRANSFER_SYNC);
                break;
            }
            case TransferData::TRANSFER_CANCELLED:
            case TransferData::TRANSFER_FAILED:
            case TransferData::TRANSFER_COMPLETED:
            {
                enableClear = true;
                break;
            }
            default:
                break;
        }
    }
    updateContextMenu(enablePause, enableResume, enableMove, enableClear, enableCancel);
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

    const auto indexes (proxy ?
                            proxy->mapSelectionToSource(selectionModel()->selection()).indexes()
                          : selectionModel()->selection().indexes());
    for (auto index : indexes)
    {
        rows.push_back(index.row());
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

void MegaTransferView::cancelSelectedClicked()
{
    onCancelClearSelection(true, false);
}

void MegaTransferView::clearSelectedClicked()
{
    onCancelClearSelection(false, true);
}

void MegaTransferView::pauseSelectedClicked()
{
    onPauseResumeSelection(true);
}

void MegaTransferView::resumeSelectedClicked()
{
    onPauseResumeSelection(false);
}
