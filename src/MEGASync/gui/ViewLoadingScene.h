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
#include <QPainter>

#include <memory>

namespace Ui {
class ViewLoadingSceneUI;
}

namespace Ui {
class ViewLoadingSceneUI;
}

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

struct MessageInfo
{
    enum ButtonType
    {
        None,
        Stop,
        Ok
    };

    QString message;
    int count = 0;
    int total = 0;
    ButtonType buttonType;
};

Q_DECLARE_METATYPE(MessageInfo)

class LoadingSceneMessageHandler : public QObject
{
    Q_OBJECT

public:
    LoadingSceneMessageHandler(Ui::ViewLoadingSceneUI* viewBaseUI, QWidget* viewBase);
    ~LoadingSceneMessageHandler();


    void hideLoadingMessage();
    void setTopParent(QWidget* widget);

    void setLoadingViewVisible(bool newLoadingViewVisible);

public slots:
    void updateMessage(std::shared_ptr<MessageInfo> info);

signals:
    void onStopPressed();
    void loadingMessageVisibilityChange(bool value);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void sendLoadingMessageVisibilityChange(bool value);
    void updateMessagePos();

    Ui::ViewLoadingSceneUI* ui;
    QWidget* mViewBase;
    QWidget* mTopParent = nullptr;
    QWidget* mFadeOutWidget;

    bool mLoadingViewVisible = false;

    std::shared_ptr<MessageInfo> mCurrentInfo;
};

class ViewLoadingSceneBase : public QObject
{
    Q_OBJECT

public:
    ViewLoadingSceneBase();

    inline void setDelayTimeToShowInMs(int newDelayTimeToShowInMs)
    {
        mDelayTimeToShowInMs = newDelayTimeToShowInMs;
    }

    LoadingSceneMessageHandler* getLoadingMessageHandler()
    {
        return mMessageHandler;
    }

    void show();
    void hide();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void showLoadingScene();
    virtual void showViewCopy();
    virtual void hideLoadingScene();

    virtual QWidget* getTopParent();

signals:
    void sceneVisibilityChange(bool value);

protected:
    QTimer mDelayTimerToShow;
    QTimer mDelayTimerToHide;
    int mDelayTimeToShowInMs;
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
        hideLoadingScene();
    }
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
        mLoadingModel(nullptr),
        mLoadingDelegate(nullptr),
        mViewLayout(nullptr)
    {}

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
        return mLoadingViewSet != LoadingViewType::NONE;
    }

    void setLoadingViewSet(LoadingViewType type)
    {
        if (mLoadingViewSet != type)
        {
            mLoadingViewSet = type;
            mMessageHandler->setLoadingViewVisible(type != LoadingViewType::NONE);
        }
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
            mLoadingView = new ViewType(mLoadingSceneUI);
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
        }

        if(state)
        {
            mDelayTimerToHide.stop();
            if(mDelayTimeToShowInMs > 0)
            {
                if(!mDelayTimerToShow.isActive())
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
            if(mDelayTimerToShow.isActive())
            {
                mDelayTimerToShow.stop();
            }

            mView->blockSignals(false);
            mView->header()->blockSignals(false);
            mView->setViewPortEventsBlocked(false);

            if(mLoadingViewSet == LoadingViewType::LOADING_VIEW)
            {
                auto delay = std::max(0ll, MIN_TIME_DISPLAYING_VIEW - (QDateTime::currentMSecsSinceEpoch()
                                                                       - mStartTime));
                delay > 0 ? mDelayTimerToHide.start(delay) : hideLoadingScene();
            }
            else
            {
                hideLoadingScene();
            }
        }
    }

    inline void hideLoadingScene() override
    {
        ViewLoadingSceneBase::hideLoadingScene();

        setLoadingViewSet(LoadingViewType::NONE);
        emit sceneVisibilityChange(false);

        mLoadingModel->setRowCount(0);
        mViewLayout->replaceWidget(mLoadingSceneUI, mView);
        hide();
        if(mWasFocused)
        {
            mView->setFocus();
        }
        mView->show();
        mView->viewport()->update();
        mLoadingDelegate->setLoading(false);
    }

protected:
    QWidget* getTopParent() override
    {
        if(!mTopParent)
        {
            mTopParent = mView->window();
            mTopParent->installEventFilter(this);


        }

        return ViewLoadingSceneBase::getTopParent();
    }

private:
    void showViewCopy() override
    {
        ViewLoadingSceneBase::showViewCopy();

        setLoadingViewSet(LoadingViewType::COPY_VIEW);

        mView->setViewPortEventsBlocked(true);
        mViewLayout->replaceWidget(mView, mLoadingSceneUI);
        show();
        mView->hide();
        mView->blockSignals(true);
        mView->header()->blockSignals(true);

        //emit sceneVisibilityChange(true);
    }

    void showLoadingScene() override
    {
        ViewLoadingSceneBase::showLoadingScene();
        setLoadingViewSet(LoadingViewType::LOADING_VIEW);

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

        mView->setViewPortEventsBlocked(true);
        mViewLayout->replaceWidget(mView, mLoadingSceneUI);
        show();
        mView->hide();
        mView->blockSignals(true);
        mView->header()->blockSignals(true);
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
class LoadingSceneView : public ViewType
{
public:
    LoadingSceneView(QWidget* parent): ViewType(parent)
    {
        mLoadingView.setView(this);
    }

    void setTopParent(QWidget* widget)
    {
        mLoadingView.setTopParent(widget);
    }

    void setViewPortEventsBlocked(bool newViewPortEventsBlocked)
    {
        mViewPortEventsBlocked = newViewPortEventsBlocked;
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
