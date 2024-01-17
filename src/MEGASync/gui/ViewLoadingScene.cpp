#include <ViewLoadingScene.h>
#include "ui_ViewLoadingScene.h"

#include <QDebug>
#include <QKeyEvent>

#include <memory>


ViewLoadingSceneBase::ViewLoadingSceneBase() :
    mDelayTimeToShowInMs(0),
    mLoadingView(nullptr),
    mLoadingViewSet(LoadingViewType::NONE),
    mTopParent(nullptr),
    ui(new Ui::ViewLoadingSceneUI())
{
    mDelayTimerToShow.setSingleShot(true);
    mDelayTimerToHide.setSingleShot(true);

    connect(&mDelayTimerToShow, &QTimer::timeout, this, &ViewLoadingSceneBase::onDelayTimerToShowTimeout);
    connect(&mDelayTimerToHide, &QTimer::timeout, this, &ViewLoadingSceneBase::onDelayTimerToHideTimeout);

    mLoadingSceneUI = new QWidget();

    mLoadingSceneUI->installEventFilter(this);
    ui->setupUi(mLoadingSceneUI);
    mLoadingSceneUI->hide();
    ui->wParentViewCopy->hide();
}

void ViewLoadingSceneBase::show()
{
    mLoadingSceneUI->show();
    mLoadingView->show();
    ui->viewLayout->addWidget(mLoadingView);
}

void ViewLoadingSceneBase::hide()
{
    mLoadingSceneUI->hide();
}

bool ViewLoadingSceneBase::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        ui->swLoadingViewContainer->resize(mLoadingSceneUI->size());
        ui->swLoadingViewContainer->move(0,0);

        if(mLoadingViewSet == LoadingViewType::COPY_VIEW)
        {
            ui->wParentViewCopy->setGeometry(QRect(QPoint(0,0), getTopParent()->size()));
        }
    }

    return QObject::eventFilter(watched, event);
}

void ViewLoadingSceneBase::onDelayTimerToShowTimeout()
{
    ui->swLoadingViewContainer->setCurrentIndex(0);
    showLoadingScene();
}

void ViewLoadingSceneBase::hideLoadingScene()
{
    if(mLoadingViewSet == LoadingViewType::COPY_VIEW)
    {
        ui->wParentViewCopy->hide();
    }
}

QWidget *ViewLoadingSceneBase::getTopParent()
{
    if(ui->wParentViewCopy->parentWidget() != mTopParent)
    {
        ui->wParentViewCopy->setParent(mTopParent);
    }

    return mTopParent;
}

void ViewLoadingSceneBase::showViewCopy()
{
    ui->wParentViewCopy->setGeometry(QRect(QPoint(0,0), getTopParent()->size()));
    ui->lParentViewCopyLabel->setPixmap(mViewPixmap);
    ui->wParentViewCopy->show();
    ui->wParentViewCopy->raise();
}

void ViewLoadingSceneBase::showLoadingScene()
{
    ui->wParentViewCopy->hide();
}
