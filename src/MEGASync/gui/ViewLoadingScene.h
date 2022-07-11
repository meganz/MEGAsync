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

class LoadingSceneDelegateBase : public QStyledItemDelegate
{
    Q_OBJECT

    const double MIN_OPACITY = 0.3;
    const double OPACITY_STEPS = 0.05;
    const double MAX_OPACITY = 1.0;
    const int    UPDATE_TIMER = 100;

public:
    explicit LoadingSceneDelegateBase(QAbstractItemView* view) : mView(view),
        mOpacitySteps(OPACITY_STEPS), mOpacity(MAX_OPACITY)
    {
        connect(&mTimer, &QTimer::timeout, this, &LoadingSceneDelegateBase::onLoadingTimerTimeout);
    }

    ~LoadingSceneDelegateBase(){}

    inline void setLoading(bool state)
    {
        updateTimer(state);
        mOpacity = MAX_OPACITY;

        mView->update();
    }

protected:
    inline void updateTimer(bool state)
    {
        state ? mTimer.start(UPDATE_TIMER) : mTimer.stop();
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

    mutable QVector<DelegateWidget*> mLoadingItems;
};

template <class DelegateWidget>
class ViewLoadingScene
{
    const uint8_t MAX_LOADING_ROWS = 20;

public:
    ViewLoadingScene() :
        mViewDelegate(nullptr),
        mView(nullptr),
        mViewModel(nullptr),
        mPotentialSourceModel(nullptr),
        mLoadingView(nullptr),
        mLoadingModel(nullptr),
        mLoadingDelegate(nullptr),
        mViewLayout(nullptr)
    {}

    ~ViewLoadingScene()
    {
        mLoadingDelegate->setLoading(false);
        mLoadingDelegate->deleteLater();
        mLoadingModel->deleteLater();
    }

    bool isLoadingViewSet() const
    {
        return mLoadingModel && mLoadingModel->rowCount() != 0;
    }

    inline void setView(QAbstractItemView* view)
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

    inline void setLoadingScene(bool state)
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
            return;
        }

        if(!mLoadingModel)
        {
            mLoadingView = new QTreeView();
            mLoadingView->setObjectName(QString::fromStdString("Loading View"));
            mLoadingView->setContentsMargins(mView->contentsMargins());
            mLoadingView->setStyleSheet(mView->styleSheet());
            mLoadingView->header()->setStretchLastSection(true);
            mLoadingView->header()->hide();
            mLoadingView->setSizePolicy(mView->sizePolicy());
            mLoadingView->setFrameStyle(QFrame::NoFrame);
            mLoadingView->setIndentation(0);

            mLoadingModel = new QStandardItemModel(mLoadingView);
            mLoadingDelegate = new LoadingSceneDelegate<DelegateWidget>(mLoadingView);
            mLoadingView->setModel(mLoadingModel);
            mLoadingView->setItemDelegate(mLoadingDelegate);

            mLoadingView->hide();
            mViewLayout->addWidget(mLoadingView);
        }

        if(state)
        {
            int visibleRows(0);

            if(mView->isVisible())
            {
                int delegateHeight(mLoadingDelegate->sizeHint(QStyleOptionViewItem(), QModelIndex()).height());

                mView->updateGeometry();
                visibleRows = mView->size().height()/delegateHeight;

                //If the rowCount is higher than the visible rows, the vertical header is visible
                if(mViewModel)
                {
                    auto rows = mViewModel->rowCount();

                    if(rows > visibleRows)
                    {
                        visibleRows++;
                    }
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
            mLoadingView->show();
        }
        else
        {
            mLoadingModel->setRowCount(0);
            mLoadingView->hide();

            mView->show();
        }

        mLoadingDelegate->setLoading(state);
    }

private:
    QAbstractItemDelegate* mViewDelegate;
    QAbstractItemView* mView;
    QPointer<QAbstractItemModel> mViewModel;
    QAbstractItemModel* mPotentialSourceModel;
    QPointer<QTreeView> mLoadingView;
    QPointer<QStandardItemModel> mLoadingModel;
    QPointer<LoadingSceneDelegate<DelegateWidget>> mLoadingDelegate;
    QLayout* mViewLayout;

    int rowCount() const
    {
        if(auto proxyModel = dynamic_cast<QSortFilterProxyModel*>(mViewModel))
        {
            return proxyModel->sourceModel()->rowCount();
        }
        else
        {
            return mViewModel->rowCount();
        }
    }
};

#endif // VIEWLOADINGSCENE_H
