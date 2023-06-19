#ifndef VIEWLOADINGSCENE_H
#define VIEWLOADINGSCENE_H

#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QPointer>
#include <QLayout>
#include <QHeaderView>
#include <QScrollBar>
#include <QDateTime>
#include <QEvent>

template <class DelegateWidget, class ViewType>
class LoadingSceneView;

class LoadingSceneDelegateBase : public QStyledItemDelegate
{
    Q_OBJECT

    const double MIN_OPACITY = 0.3;
    const double OPACITY_STEPS = 0.05;
    const double MAX_OPACITY = 1.0;
    const int    UPDATE_TIMER = 100;

public:
    explicit LoadingSceneDelegateBase(QAbstractItemView* view) :
        QStyledItemDelegate(view),
        mView(view),
        mOpacitySteps(OPACITY_STEPS),
        mOpacity(MAX_OPACITY)
    {
    }
    QWidget *createEditor(QWidget *,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const override
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
        if(state)
        {
            connect(&mTimer, &QTimer::timeout, this, &LoadingSceneDelegateBase::onLoadingTimerTimeout);
            mTimer.start(UPDATE_TIMER);
        }
        else
        {
            disconnect(&mTimer, &QTimer::timeout, this, &LoadingSceneDelegateBase::onLoadingTimerTimeout);
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

        if(currentClass && mView)
        {
            if(mOpacity < MIN_OPACITY)
            {
                mOpacitySteps = OPACITY_STEPS;
                mOpacity = MIN_OPACITY;
            }
            else if(mOpacity > MAX_OPACITY)
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

template <class DelegateWidget>
class LoadingSceneDelegate : public LoadingSceneDelegateBase
{
public:
    explicit LoadingSceneDelegate(QAbstractItemView* view) : LoadingSceneDelegateBase(view)
    {}

    inline QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
    {
        return DelegateWidget::widgetSize();
    }

protected:
    inline void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->fillRect(option.rect, Qt::white);

        auto pos (option.rect.topLeft());
        auto width (option.rect.width());
        auto height (option.rect.height());

        auto loadingItem = getLoadingWidget(index,option.rect.size());

        if(!loadingItem)
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
        loadingItem->render(painter,QPoint(0,0), QRegion(0, 0, width, height));

        painter->restore();
    }

private:
    inline DelegateWidget* getLoadingWidget(const QModelIndex &index, const QSize &size) const
    {
        auto nbRowsMaxInView(1);
        if(size.height() > 0)
        {
            nbRowsMaxInView = getView()->height() / size.height() + 1;
        }
        auto row (index.row() % nbRowsMaxInView);

        DelegateWidget* item(nullptr);

        if(row >= mLoadingItems.size())
        {
           item = new DelegateWidget(getView());
           mLoadingItems.append(item);
        }
        else
        {
            item = mLoadingItems.at(row);
        }

        return item;
    }

    //These items are removed when the view is removed
    mutable QVector<DelegateWidget*> mLoadingItems;
};

class ViewLoadingSceneBase : public QObject
{
    Q_OBJECT

 public:
    ViewLoadingSceneBase() :
        mDelayTimeToShowInMs(0)
    {
        mDelayTimerToShow.setSingleShot(true);
        mDelayTimerToHide.setSingleShot(true);

        connect(&mDelayTimerToShow, &QTimer::timeout, this, &ViewLoadingSceneBase::onDelayTimerToShowTimeout);
        connect(&mDelayTimerToHide, &QTimer::timeout, this, &ViewLoadingSceneBase::onDelayTimerToHideTimeout);
    }

    inline void setDelayTimeToShowInMs(int newDelayTimeToShowInMs)
    {
        mDelayTimeToShowInMs = newDelayTimeToShowInMs;
    }

signals:
    void sceneVisibilityChange(bool value);

protected:
    QTimer mDelayTimerToShow;
    QTimer mDelayTimerToHide;
    int mDelayTimeToShowInMs;


private slots:
    void onDelayTimerToShowTimeout()
    {
        showLoadingScene();
    }

    void onDelayTimerToHideTimeout()
    {
        hideLoadingScene();
    }

private:
    virtual void showLoadingScene() = 0;
    virtual void hideLoadingScene() = 0;
};

template <class DelegateWidget, class ViewType>
class ViewLoadingScene : public ViewLoadingSceneBase
{
    const uint8_t MAX_LOADING_ROWS = 20;
    const long long MIN_TIME_DISPLAYING_VIEW = 350;

public:
    ViewLoadingScene() :
        ViewLoadingSceneBase(),
        mViewDelegate(nullptr),
        mView(nullptr),
        mViewModel(nullptr),
        mPotentialSourceModel(nullptr),
        mLoadingView(nullptr),
        mLoadingModel(nullptr),
        mLoadingDelegate(nullptr),
        mViewLayout(nullptr),
        mLoadingViewSet(false)
    {
    }

    ~ViewLoadingScene()
    {
        if(mLoadingDelegate)
        {
            mLoadingDelegate->deleteLater();
        }
        if(mLoadingModel)
        {
            mLoadingModel->deleteLater();
        }
    }

    bool isLoadingViewSet() const
    {
        return mLoadingViewSet;
    }

    inline void setView(LoadingSceneView<DelegateWidget, ViewType>* view)
    {
        mView = view;
        mViewDelegate = view->itemDelegate();
        mViewModel = view->model();

        auto parentWidget = mView->parentWidget();
        if(parentWidget && parentWidget->layout())
        {
            mViewLayout = parentWidget->layout();
        }
    }

    inline void toggleLoadingScene(bool state)
    {
        if(!mView)
        {
            return;
        }

        if(state && isLoadingViewSet())
        {
            return;
        }
        else if(!state && !isLoadingViewSet())
        {
            if(mDelayTimerToShow.isActive())
            {
                mDelayTimerToShow.stop();
            }
            return;
        }

        if(!mLoadingModel)
        {
            mLoadingView = new ViewType();
            mLoadingView->setObjectName(QString::fromStdString("Loading View"));
            mLoadingView->setContentsMargins(mView->contentsMargins());
            mLoadingView->setStyleSheet(mView->styleSheet());
            mLoadingView->header()->setStretchLastSection(true);
            mLoadingView->header()->hide();
            mLoadingView->setSizePolicy(mView->sizePolicy());
            mLoadingView->setFrameStyle(QFrame::NoFrame);
            mLoadingView->setIndentation(0);
            mLoadingView->setSelectionMode(QAbstractItemView::NoSelection);
            mLoadingModel = new QStandardItemModel(mLoadingView);
            mLoadingDelegate = new LoadingSceneDelegate<DelegateWidget>(mLoadingView);
            mLoadingView->setModel(mLoadingModel);
            mLoadingView->setItemDelegate(mLoadingDelegate);

            mLoadingView->hide();
            mViewLayout->addWidget(mLoadingView);
        }

        if(state)
        {
            mDelayTimerToHide.stop();
            if(mDelayTimeToShowInMs > 0)
            {
                if(!mDelayTimerToShow.isActive())
                {
                    mDelayTimerToShow.start(mDelayTimeToShowInMs);
                }
            }
            else
            {
                showLoadingScene();
            }
        }
        else
        {
            mView->blockSignals(false);
            mView->header()->blockSignals(false);
            mView->setViewPortEventsBlocked(false);

            auto delay = std::max(0ll, MIN_TIME_DISPLAYING_VIEW - (QDateTime::currentMSecsSinceEpoch()
                                                - mStartTime));
            delay > 0 ? mDelayTimerToHide.start(delay) : hideLoadingScene();
        }
    }

    inline void hideLoadingScene() override
    {

        mLoadingViewSet = false;
        emit sceneVisibilityChange(false);

        mLoadingModel->setRowCount(0);
        mLoadingView->hide();
        if(mWasFocused)
        {
            mView->setFocus();
        }
        mViewLayout->replaceWidget(mLoadingView, mView);
        mView->show();
        mView->viewport()->update();
        mLoadingDelegate->setLoading(false);
    }

private:
    void showLoadingScene() override
    {
        int visibleRows(0);

        if(mView->isVisible())
        {
            mWasFocused = mView->hasFocus();
            int delegateHeight(mLoadingDelegate->sizeHint(QStyleOptionViewItem(), QModelIndex()).height());

            mView->updateGeometry();
            visibleRows = mView->size().height()/delegateHeight + 1;

            //If the vertical header is visible, add one row to the loading model to show the vertical scroll
            if(mViewModel)
            {
                mView->verticalScrollBar()->isVisible() ? mLoadingView->verticalScrollBar()->show() : mLoadingView->verticalScrollBar()->hide();
            }

            if(visibleRows > MAX_LOADING_ROWS)
            {
                visibleRows = MAX_LOADING_ROWS;
            }
        }
        else
        {
            visibleRows = MAX_LOADING_ROWS;
        }

        for(int row = 0; row < visibleRows; ++row)
        {
            mLoadingModel->appendRow(new QStandardItem());
        }

        mView->hide();
        mView->blockSignals(true);
        mView->header()->blockSignals(true);
        mView->setViewPortEventsBlocked(true);
        mLoadingView->show();
        mViewLayout->replaceWidget(mView, mLoadingView);
        mStartTime = QDateTime::currentMSecsSinceEpoch();
        mLoadingDelegate->setLoading(true);

        mLoadingViewSet = true;
        emit sceneVisibilityChange(true);
    }

    QAbstractItemDelegate* mViewDelegate;
    LoadingSceneView<DelegateWidget, ViewType>* mView;
    QPointer<QAbstractItemModel> mViewModel;
    QAbstractItemModel* mPotentialSourceModel;
    QPointer<QTreeView> mLoadingView;
    QPointer<QStandardItemModel> mLoadingModel;
    QPointer<LoadingSceneDelegate<DelegateWidget>> mLoadingDelegate;
    QLayout* mViewLayout;
    qint64 mStartTime;
    bool mLoadingViewSet;
    bool mWasFocused;
};

template<class DelegateWidget, class ViewType>
class LoadingSceneView : public ViewType
{
public:
    LoadingSceneView(QWidget* parent): ViewType(parent)
    {
        mLoadingView.setView(this);
    }

    void setViewPortEventsBlocked(bool newViewPortEventsBlocked)
    {
        mViewPortEventsBlocked = newViewPortEventsBlocked;
    }

    ViewLoadingScene<DelegateWidget, ViewType>& loadingView()
    {
        return mLoadingView;
    }

protected:
    bool viewportEvent(QEvent *event) override
    {
        if(mViewPortEventsBlocked)
        {
            event->accept();
            return true;
        }

        return ViewType::viewportEvent(event);
    }

private:
    bool mViewPortEventsBlocked = false;
    ViewLoadingScene<DelegateWidget, ViewType> mLoadingView;
};

#endif // VIEWLOADINGSCENE_H
