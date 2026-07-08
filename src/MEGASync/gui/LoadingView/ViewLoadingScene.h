#ifndef VIEWLOADINGSCENE_H
#define VIEWLOADINGSCENE_H

#include "ILoadingViewModel.h"
#include "TokenParserWidgetManager.h"

#include <QDateTime>
#include <QEvent>
#include <QHash>
#include <QHeaderView>
#include <QItemSelection>
#include <QLayout>
#include <QPainter>
#include <QPointer>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QTreeView>
#include <QWidget>

#include <memory>

namespace Ui
{
class ViewLoadingSceneUI;
}

namespace Ui
{
class ViewLoadingSceneUI;
}

template<class DelegateWidget, class ViewType>
class LoadingSceneView;

class LoadingSceneDelegateBase: public QStyledItemDelegate
{
    Q_OBJECT

    const double MIN_OPACITY = 0.3;
    const double OPACITY_STEPS = 0.05;
    const double MAX_OPACITY = 1.0;
    const int UPDATE_TIMER = 100;

public:
    explicit LoadingSceneDelegateBase(QAbstractItemView* view):
        QStyledItemDelegate(view),
        mView(view),
        mOpacitySteps(OPACITY_STEPS),
        mOpacity(MAX_OPACITY)
    {}

    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override
    {
        return nullptr;
    }

    ~LoadingSceneDelegateBase()
    {
        updateTimer(false);
    }

    inline void setLoading(bool state)
    {
        updateTimer(state);
        mOpacity = MAX_OPACITY;
        mView->update();
    }

protected:
    inline void updateTimer(bool state)
    {
        if (state)
        {
            connect(&mTimer,
                    &QTimer::timeout,
                    this,
                    &LoadingSceneDelegateBase::onLoadingTimerTimeout);
            mTimer.start(UPDATE_TIMER);
        }
        else
        {
            disconnect(&mTimer,
                       &QTimer::timeout,
                       this,
                       &LoadingSceneDelegateBase::onLoadingTimerTimeout);
            mTimer.stop();
        }
    }

    inline double getOpacity() const
    {
        return mOpacity;
    }

    inline QAbstractItemView* getView() const
    {
        return mView;
    }

private slots:

    void onLoadingTimerTimeout()
    {
        QPointer<LoadingSceneDelegateBase> currentClass(this);

        if (currentClass && mView)
        {
            if (mOpacity < MIN_OPACITY)
            {
                mOpacitySteps = OPACITY_STEPS;
                mOpacity = MIN_OPACITY;
            }
            else if (mOpacity > MAX_OPACITY)
            {
                mOpacitySteps = -OPACITY_STEPS;
                mOpacity = MAX_OPACITY;
            }
            else
            {
                mOpacity += mOpacitySteps;
            }

            mView->viewport()->update();
        }
    }

private:
    QTimer mTimer;
    QPointer<QAbstractItemView> mView;
    double mOpacitySteps;
    double mOpacity;
};

template<class DelegateWidget>
class LoadingSceneDelegate: public LoadingSceneDelegateBase
{
public:
    explicit LoadingSceneDelegate(QAbstractItemView* view):
        LoadingSceneDelegateBase(view)
    {}

    inline QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override
    {
        return DelegateWidget::widgetSize();
    }

protected:
    inline void paint(QPainter* painter,
                      const QStyleOptionViewItem& option,
                      const QModelIndex& index) const override
    {
        auto pos(option.rect.topLeft());
        auto width(option.rect.width());
        auto height(option.rect.height());

        auto loadingItem = getLoadingWidget(index, option.rect.size());

        if (!loadingItem)
        {
            return;
        }

        // Move if position changed
        if (loadingItem->pos() != pos)
        {
            loadingItem->move(pos);
        }

        // Resize if window resized
        if (loadingItem->width() != width)
        {
            loadingItem->resize(width, height);
        }

        painter->save();

        painter->setOpacity(getOpacity());
        painter->translate(pos);
        loadingItem->render(painter, QPoint(0, 0), QRegion(0, 0, width, height));

        painter->restore();
    }

private:
    inline DelegateWidget* getLoadingWidget(const QModelIndex& index, const QSize& size) const
    {
        auto nbRowsMaxInView(1);
        if (size.height() > 0)
        {
            nbRowsMaxInView = getView()->height() / size.height() + 1;
        }
        auto row(index.row() % nbRowsMaxInView);

        DelegateWidget* item(nullptr);

        if (row >= mLoadingItems.size())
        {
            item = new DelegateWidget(getView());

            TokenParserWidgetManager::instance()->applyCurrentTheme(item);

            // Setting again its own parent will tell the widget that the stylesheet needs to be
            // reloaded
            item->setParent(item->parentWidget(), item->windowFlags());

            // Refresh completely the widget
            item->show();
            TokenParserWidgetManager::instance()->polish(item);
            item->hide();
            mLoadingItems.append(item);
        }
        else
        {
            item = mLoadingItems.at(row);
        }

        return item;
    }

    // These items are removed when the view is removed
    mutable QVector<DelegateWidget*> mLoadingItems;
};

struct MessageInfo
{
    enum ButtonType
    {
        NONE,
        STOP,
        OK
    };

    QString message;
    int count = 0;
    int total = 0;
    ButtonType buttonType;
};

Q_DECLARE_METATYPE(MessageInfo)

class ViewLoadingMessage;

class LoadingSceneMessageHandler: public QObject
{
    Q_OBJECT

public:
    LoadingSceneMessageHandler(Ui::ViewLoadingSceneUI* viewBaseUI, QWidget* viewBase);
    ~LoadingSceneMessageHandler();

    bool needsAnswerFromUser() const;

    void hideLoadingMessage();
    void setTopParent(QWidget* widget);

    void setLoadingViewVisible(bool newLoadingViewVisible);

public slots:
    void updateMessage(std::shared_ptr<MessageInfo> info);

signals:
    void buttonPressed(MessageInfo::ButtonType);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onButtonPressed(int buttonType);

private:
    void checkLoadingMessageVisibility();
    void createLoadingMessage();

    Ui::ViewLoadingSceneUI* ui;
    QWidget* mViewBase;
    QWidget* mTopParent = nullptr;

    QPointer<ViewLoadingMessage> mLoadingMessage;

    bool mLoadingViewVisible = false;
};

class ViewLoadingSceneBase: public QObject
{
    Q_OBJECT

public:
    ViewLoadingSceneBase();

    inline void setDelayTimeToShowInMs(int newDelayTimeToShowInMs)
    {
        mDelayTimeToShowInMs = newDelayTimeToShowInMs;
    }

    LoadingSceneMessageHandler* getLoadingMessageHandler() const
    {
        return mMessageHandler;
    }

    void show();
    void hide();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void showLoadingScene();
    virtual void showViewCopy();
    virtual void hideLoadingScene();
    void hideViewCopy();

    virtual QWidget* getTopParent();

    // Hooks to defer hiding while the proxy is still sorting in another thread.
    // Implemented by the template, which knows the view's model.
    virtual bool isViewModelBusy() const
    {
        return false;
    }

signals:
    void sceneVisibilityChange(bool value);

protected:
    QTimer mDelayTimerToShow;
    QTimer mDelayTimerToHide;
    int mDelayTimeToShowInMs = 0;
    QPointer<QTreeView> mLoadingView;
    QWidget* mLoadingSceneUI;
    Ui::ViewLoadingSceneUI* ui;
    QPixmap mViewPixmap;
    QWidget* mTopParent;

    enum LoadingViewType
    {
        NONE,
        COPY_VIEW,
        LOADING_VIEW
    };

    LoadingViewType mLoadingViewSet;
    LoadingSceneMessageHandler* mMessageHandler;

private slots:
    void onDelayTimerToShowTimeout();

    void onDelayTimerToHideTimeout()
    {
        // The model is still working in another thread: keep the loading view.
        // The model will request the hide again when it finishes.
        if (isViewModelBusy())
        {
            return;
        }

        hideLoadingScene();
    }
};

template<class DelegateWidget, class ViewType>
class ViewLoadingScene: public ViewLoadingSceneBase
{
    const uint8_t MAX_LOADING_ROWS = 50;
    const int MIN_TIME_DISPLAYING_VIEW = 350;

public:
    ViewLoadingScene():
        ViewLoadingSceneBase(),
        mViewDelegate(nullptr),
        mView(nullptr),
        mViewModel(nullptr),
        mLoadingModel(nullptr),
        mLoadingDelegate(nullptr),
        mViewLayout(nullptr)
    {}

    ~ViewLoadingScene() {}

    bool isLoadingViewSet() const
    {
        return mLoadingViewSet != LoadingViewType::NONE;
    }

    bool isLoadingViewWaitingForUserAnswer() const
    {
        return getLoadingMessageHandler() ? getLoadingMessageHandler()->needsAnswerFromUser() :
                                            false;
    }

    void setLoadingViewSet(LoadingViewType type)
    {
        if (mLoadingViewSet != type)
        {
            mLoadingViewSet = type;
        }
    }

    inline void setView(LoadingSceneView<DelegateWidget, ViewType>* view)
    {
        mView = view;
        mViewDelegate = view->itemDelegate();
        mViewModel = view->model();

        auto parentWidget = mView->parentWidget();
        if (parentWidget && parentWidget->layout())
        {
            mViewLayout = parentWidget->layout();
        }
    }

    inline void toggleLoadingScene(bool state)
    {
        if (!mView)
        {
            return;
        }

        if (state)
        {
            // A deferred hide (MIN_TIME_DISPLAYING_VIEW) still pending must never fire once
            // the UI is blocked again: it would reattach the view while the newly started
            // concurrent sort/filter job is mutating the proxy -> use-after-free. Cancel it
            // even when the loading scene is already visible (early return below).
            mDelayTimerToHide.stop();
        }

        if (state && isLoadingViewSet())
        {
            return;
        }
        else if (!state && !isLoadingViewSet())
        {
            if (mDelayTimerToShow.isActive())
            {
                mDelayTimerToShow.stop();
            }
            return;
        }

        // Don´t close the loading view if we need interaction from the user (like clicking ok or
        // stop...)
        if (!state && getLoadingMessageHandler() &&
            getLoadingMessageHandler()->needsAnswerFromUser())
        {
            return;
        }

        // Don´t close the loading view if the model is still working in another thread;
        // reattaching the view now would race the worker over the proxy mapping. The
        // model will request the hide again when it finishes.
        if (!state && isViewModelBusy())
        {
            return;
        }

        if (!mLoadingModel)
        {
            mLoadingView = new ViewType(mLoadingSceneUI);
            mLoadingView->setObjectName(QString::fromStdString("Loading View"));
            mLoadingView->setContentsMargins(mView->contentsMargins());
            mLoadingView->header()->setStretchLastSection(true);
            mLoadingView->header()->hide();
            mLoadingView->setSizePolicy(mView->sizePolicy());
            mLoadingView->setFrameStyle(QFrame::NoFrame);
            mLoadingView->setIndentation(0);
            mLoadingView->setSelectionMode(QAbstractItemView::NoSelection);
            mLoadingView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            mLoadingModel = new QStandardItemModel(mLoadingView);
            mLoadingDelegate = new LoadingSceneDelegate<DelegateWidget>(mLoadingView);
            mLoadingView->setModel(mLoadingModel);
            mLoadingView->setItemDelegate(mLoadingDelegate);

            mLoadingView->setStyleSheet(
                QLatin1String("QTreeView {\n"
                              "  background-color: #000000; /*colorToken.page-background*/\n"
                              "  color: #000000; /*colorToken.text-primary*/\n"
                              "  border: none;\n"
                              "}\n"));
            TokenParserWidgetManager::instance()->registerWidgetForTheming(mLoadingView);
        }

        if (state)
        {
            if (mDelayTimeToShowInMs > 0)
            {
                if (!mDelayTimerToShow.isActive())
                {
                    mViewPixmap = getTopParent()->grab();
                    mDelayTimerToShow.start(mDelayTimeToShowInMs);
                    showViewCopy();
                }
            }
            else
            {
                showLoadingScene();
            }
        }
        else
        {
            if (mDelayTimerToShow.isActive())
            {
                mDelayTimerToShow.stop();
            }

            if (mLoadingViewSet == LoadingViewType::LOADING_VIEW)
            {
                qint64 timeFromStart(QDateTime::currentMSecsSinceEpoch() - mStartTime);
                int delay = std::max(0, MIN_TIME_DISPLAYING_VIEW - static_cast<int>(timeFromStart));
                delay > 0 ? mDelayTimerToHide.start(delay) : hideLoadingScene();
            }
            else
            {
                hideLoadingScene();
            }
        }
    }

public:
    inline void hideLoadingScene() override
    {
        const bool copyWasShown = (mLoadingViewSet == LoadingViewType::COPY_VIEW);

        setLoadingViewSet(LoadingViewType::NONE);

        mLoadingModel->setRowCount(0);
        mViewLayout->replaceWidget(mLoadingSceneUI, mView);
        hide();
        mView->setUpdatesEnabled(false);
        mView->setViewPortEventsBlocked(false);
        mView->restoreLoadingViewState();
        // Notify AFTER the model is reattached, so consumers' post-load logic (e.g.
        // selectPendingIndexes) operates on the attached model and not the detached one.
        emit sceneVisibilityChange(false);
        if (mWasFocused)
        {
            mView->setFocus();
        }
        // showViewCopy/showLoadingScene block viewport events for every view (detaching
        // or not); re-enable here unconditionally or the view stays blank.
        mView->show();
        mView->applyLoadingViewScroll();
        mView->setUpdatesEnabled(true);
        // Paint the restored view synchronously so a real frame exists underneath the (raised)
        // copy BEFORE we lift it. update() only schedules a paint -> would leave a blank gap.
        mView->viewport()->repaint();
        mLoadingDelegate->setLoading(false);

        // Lift the copy snapshot LAST, now that the restored view is already painted behind it.
        // Hiding it at the top (as before) left a ~5ms blank gap that showed as a flicker.
        if (copyWasShown)
        {
            hideViewCopy();
        }
    }

protected:
    QWidget* getTopParent() override
    {
        if (!mTopParent)
        {
            mTopParent = mView->window();
            mTopParent->installEventFilter(this);
        }

        return ViewLoadingSceneBase::getTopParent();
    }

    bool isViewModelBusy() const override
    {
        if (!mView)
        {
            return false;
        }

        auto workingModel = dynamic_cast<const ILoadingViewModel*>(mView->currentModel());
        return workingModel && workingModel->isWorking();
    }

private:
    void showViewCopy() override
    {
        ViewLoadingSceneBase::showViewCopy();

        mView->saveLoadingViewState();
        setLoadingViewSet(LoadingViewType::COPY_VIEW);

        mViewLayout->replaceWidget(mView, mLoadingSceneUI);
        show();
        mView->hide();
    }

    void showLoadingScene() override
    {
        ViewLoadingSceneBase::showLoadingScene();
        mView->saveLoadingViewState();
        setLoadingViewSet(LoadingViewType::LOADING_VIEW);

        int visibleRows(0);

        if (mView->isVisible())
        {
            mWasFocused = mView->hasFocus();
            int delegateHeight(
                mLoadingDelegate->sizeHint(QStyleOptionViewItem(), QModelIndex()).height());

            mView->updateGeometry();
            visibleRows = mView->size().height() / delegateHeight + 1;

            // If the vertical header is visible, add one row to the loading model to show the
            // vertical scroll
            if (mViewModel)
            {
                mView->verticalScrollBar()->isVisible() ?
                    mLoadingView->verticalScrollBar()->show() :
                    mLoadingView->verticalScrollBar()->hide();
            }

            if (visibleRows > MAX_LOADING_ROWS)
            {
                visibleRows = MAX_LOADING_ROWS;
            }
        }
        else
        {
            visibleRows = MAX_LOADING_ROWS;
        }

        for (int row = 0; row < visibleRows; ++row)
        {
            mLoadingModel->appendRow(new QStandardItem());
        }

        mViewLayout->replaceWidget(mView, mLoadingSceneUI);
        show();
        mView->hide();
        mStartTime = QDateTime::currentMSecsSinceEpoch();
        mLoadingDelegate->setLoading(true);

        emit sceneVisibilityChange(true);
    }

    QAbstractItemDelegate* mViewDelegate;
    LoadingSceneView<DelegateWidget, ViewType>* mView;
    QPointer<QAbstractItemModel> mViewModel;
    QPointer<QStandardItemModel> mLoadingModel;
    QPointer<LoadingSceneDelegate<DelegateWidget>> mLoadingDelegate;
    QLayout* mViewLayout;
    qint64 mStartTime;

    bool mWasFocused;
};

template<class DelegateWidget, class ViewType>
class LoadingSceneView: public ViewType
{
public:
    LoadingSceneView(QWidget* parent):
        ViewType(parent)
    {
        mLoadingView.setView(this);
    }

    void setRootIndex(const QModelIndex& index) override
    {
        // setModel() internally calls reset() -> setRootIndex(QModelIndex()). That is
        // our own detach/reattach, not a navigation: let the base handle it but DO NOT
        // touch the saved state (it would clobber the real saved root).
        if (!mSwappingModel && mDetachedModel)
        {
            // The model is detached while the loading scene is shown, so the view cannot
            // take this root now. Record it (as a source persistent index) so the pending
            // restore applies THIS root instead of the stale one captured when detaching.
            auto* proxy = qobject_cast<QSortFilterProxyModel*>(mDetachedModel.data());
            mSavedRootIndex = (proxy && index.isValid()) ?
                                  QPersistentModelIndex(proxy->mapToSource(index)) :
                                  QPersistentModelIndex();
            return;
        }

        ViewType::setRootIndex(index);
    }

    // Column visibility is owned by the consumer, which re-applies it on every model reattach
    // (the loading scene emits sceneVisibilityChange(false) once the model is back). While the
    // model is detached the header has no sections, so this is a harmless no-op; we no longer
    // buffer the change (the buffer could desync and leave columns wrongly shown/hidden).
    void setColumnHidden(int column, bool hide)
    {
        ViewType::setColumnHidden(column, hide);
    }

    // Called by the loading scene when it shows/hides. When detachModelDuringLoading()
    // is enabled, the model is detached from the view while loading so the view never
    // queries the proxy while it is being re-sorted in another thread (which produced
    // broken layouts). State (scroll, expanded rows, selection, current) is captured as
    // SOURCE persistent indexes -> survives the proxy re-sort and the detach, and is
    // generic for any QSortFilterProxyModel.
    virtual void saveLoadingViewState()
    {
        if (!detachModelDuringLoading() || mDetachedModel || !this->model())
        {
            return; // disabled or already detached (idempotent)
        }

        ViewType::blockSignals(true);
        ViewType::header()->blockSignals(true);
        setViewPortEventsBlocked(true);

        // setModel resets the header; save its layout (column sizes, hidden columns,
        // resize modes, visual order) so the columns don't come back compact.
        mSavedHeaderState = ViewType::header()->saveState();

        auto* proxy = qobject_cast<QSortFilterProxyModel*>(this->model());

        // Fresh load cycle: any external scroll override from a previous cycle is stale.
        mScrollHandledExternally = false;

        mSavedHasVScroll = this->verticalScrollBar()->isVisible();
        mSavedVScroll = mSavedHasVScroll ? this->verticalScrollBar()->value() : 0;
        mSavedHasHScroll = this->horizontalScrollBar()->isVisible();
        mSavedHScroll = mSavedHasHScroll ? this->horizontalScrollBar()->value() : 0;

        // The selection contents are not saved here: the selection model object is preserved
        // (mPreservedSelectionModel) and reattached as-is in restoreLoadingViewState. Since the
        // model is only detached/reattached (not reset) during loading, its selection survives
        // the swap, so no manual save/restore of selected indexes is needed.

        mSavedExpanded.clear();
        if (proxy)
        {
            collectExpandedSource(this->rootIndex(), proxy, mSavedExpanded);
        }

        mSavedRootIndex = proxy ? QPersistentModelIndex(proxy->mapToSource(this->rootIndex())) :
                                  QPersistentModelIndex();

        // Preserve the selection model object so external connections to it survive the
        // model swap (the view would otherwise create a brand new one).
        mPreservedSelectionModel = this->selectionModel();
        if (mPreservedSelectionModel)
        {
            mPreservedSelectionModel->setParent(this->window());
        }

        mDetachedModel = this->model();
        mSwappingModel = true;
        this->setModel(nullptr);
        mSwappingModel = false;
    }

    virtual void restoreLoadingViewState()
    {
        if (!mDetachedModel)
        {
            return;
        }

        // Unblock only after the whole reattach (setModel + root + expand + select) so
        // none of those intermediate changes propagate (avoids the breadcrumb flicker).
        ViewType::header()->blockSignals(false);
        ViewType::blockSignals(false);

        mSwappingModel = true;
        this->setModel(mDetachedModel);
        mSwappingModel = false;
        mDetachedModel = nullptr;

        // Use the model actually set on the view. If it changed while loading (e.g. a
        // navigation swapped/reset it), the preserved selection model and the saved
        // indexes belong to a different model -> skip them instead of warning.
        auto* proxy = qobject_cast<QSortFilterProxyModel*>(this->model());

        if (!mSavedHeaderState.isEmpty())
        {
            ViewType::header()->restoreState(mSavedHeaderState);
            mSavedHeaderState.clear();
        }

        // Column visibility is not restored from mSavedHeaderState here: the consumer re-applies
        // it after reattach (on sceneVisibilityChange(false)), which is the single source of truth.

        if (mPreservedSelectionModel)
        {
            if (mPreservedSelectionModel->model() == this->model())
            {
                this->setSelectionModel(mPreservedSelectionModel);
            }

            mPreservedSelectionModel = nullptr;
        }

        if (proxy)
        {
            // Restore the view's root (folder the user navigated into) before expansion
            // and selection, so those operate on the right subtree.
            const QModelIndex root =
                mSavedRootIndex.isValid() ? proxy->mapFromSource(mSavedRootIndex) : QModelIndex();
            this->setRootIndex(root);

            // A pending "expand all" requested while detached: now that the model is back
            // (with the freshly loaded rows) apply it here.
            if (mExpandAllOnRestore)
            {
                this->expandAll();
            }
            else
            {
                for (const auto& src: mSavedExpanded)
                {
                    const auto idx = src.isValid() ? proxy->mapFromSource(src) : QModelIndex();
                    if (idx.isValid())
                    {
                        this->setExpanded(idx, true);
                    }
                }
            }
        }

        mSavedExpanded.clear();
        mExpandAllOnRestore = false;
        mSavedRootIndex = QPersistentModelIndex();
    }

    // Applied AFTER the view is shown: the scrollbars only have a valid range once the
    // view is visible, so setting it earlier would clamp a high value back to 0.
    void applyLoadingViewScroll()
    {
        this->doItemsLayout(); // force range computation now that the view is visible

        // The post-load selection pass already scrolled to a navigated/selected node; keep that
        // position instead of restoring the pre-load scroll (which would push a deep target out of
        // view).
        if (!mScrollHandledExternally)
        {
            if (mSavedHasVScroll)
            {
                this->verticalScrollBar()->setValue(mSavedVScroll);
            }
            if (mSavedHasHScroll)
            {
                this->horizontalScrollBar()->setValue(mSavedHScroll);
            }
        }

        mScrollHandledExternally = false;
        mSavedHasVScroll = false;
        mSavedHasHScroll = false;
        mSavedVScroll = 0;
        mSavedHScroll = 0;
    }

    void setTopParent(QWidget* widget)
    {
        mLoadingView.setTopParent(widget);
    }

    // Expands all rows once the data is ready. While the model is detached during loading,
    // expandAll() would be a no-op, so the request is deferred and applied on reattach
    // (restoreLoadingViewState).
    void expandAllWhenReady()
    {
        if (mDetachedModel)
        {
            mExpandAllOnRestore = true;
        }
        else
        {
            this->expandAll();
        }
    }

    void setViewPortEventsBlocked(bool newViewPortEventsBlocked)
    {
        mViewPortEventsBlocked = newViewPortEventsBlocked;
    }

    // When the post-load selection pass scrolls the view to a navigated/selected node (e.g. a
    // searched node deep in the tree), it calls this so applyLoadingViewScroll() keeps that
    // position instead of restoring the pre-load scroll captured in saveLoadingViewState(). Reset
    // at the start of every load so it only affects the current reattach cycle.
    void markScrollHandledExternally()
    {
        mScrollHandledExternally = true;
    }

    // Model currently driving the view: the detached one while the loading scene is
    // shown (the view's own model() is null then), the attached one otherwise.
    QAbstractItemModel* currentModel() const
    {
        return mDetachedModel ? mDetachedModel.data() : this->model();
    }

    ViewLoadingScene<DelegateWidget, ViewType>& loadingView()
    {
        return mLoadingView;
    }

    LoadingSceneMessageHandler* getLoadingMessageHandler()
    {
        return mLoadingView.getLoadingMessageHandler();
    }

protected:
    bool viewportEvent(QEvent* event) override
    {
        if (mViewPortEventsBlocked)
        {
            event->accept();
            return true;
        }

        return ViewType::viewportEvent(event);
    }

    // Views that must detach their model during loading override this to return true.
    virtual bool detachModelDuringLoading() const
    {
        return true;
    }

private:
    void collectExpandedSource(const QModelIndex& parent,
                               QSortFilterProxyModel* proxy,
                               QList<QPersistentModelIndex>& out) const
    {
        const int rows = proxy->rowCount(parent);
        for (int row = 0; row < rows; ++row)
        {
            const auto idx = proxy->index(row, 0, parent);
            if (this->isExpanded(idx))
            {
                out.append(QPersistentModelIndex(proxy->mapToSource(idx)));
                collectExpandedSource(idx, proxy, out);
            }
        }
    }

    bool mViewPortEventsBlocked = false;
    ViewLoadingScene<DelegateWidget, ViewType> mLoadingView;
    QPointer<QAbstractItemModel> mDetachedModel;
    QPointer<QItemSelectionModel> mPreservedSelectionModel;
    QList<QPersistentModelIndex> mSavedExpanded;
    bool mExpandAllOnRestore = false;
    QPersistentModelIndex mSavedRootIndex;
    bool mSwappingModel = false;
    QByteArray mSavedHeaderState;
    bool mSavedHasVScroll = false;
    int mSavedVScroll = 0;
    bool mSavedHasHScroll = false;
    int mSavedHScroll = 0;
    bool mScrollHandledExternally = false;
};

#endif // VIEWLOADINGSCENE_H
