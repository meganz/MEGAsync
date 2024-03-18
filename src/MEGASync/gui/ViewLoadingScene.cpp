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
    mMessageHandler = new LoadingSceneMessageHandler(ui, mLoadingSceneUI);

    mLoadingSceneUI->installEventFilter(this);
    ui->setupUi(mLoadingSceneUI);
    mLoadingSceneUI->hide();
    ui->wMessageContainer->hide();
    ui->wParentViewCopy->hide();

    connect(ui->bStopButton, &QPushButton::clicked, mMessageHandler, &LoadingSceneMessageHandler::onStopPressed);
}

void ViewLoadingSceneBase::show()
{
    ui->wMessageContainer->hide();
    mLoadingSceneUI->show();
    mLoadingView->show();
    ui->swLoadingViewContainer->stackUnder(ui->wMessageContainer);
    ui->viewLayout->addWidget(mLoadingView);
}

void ViewLoadingSceneBase::hide()
{
    mLoadingSceneUI->hide();
    mMessageHandler->hideLoadingMessage();
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
    if(!ui->wMessageContainer->parent())
    {
        ui->wMessageContainer->setParent(mTopParent);
    }

    if(ui->wParentViewCopy->parentWidget() != mTopParent)
    {
        ui->wParentViewCopy->setParent(mTopParent);
    }

    mMessageHandler->setTopParent(mTopParent);

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

LoadingSceneMessageHandler::LoadingSceneMessageHandler(Ui::ViewLoadingSceneUI *viewBaseUI, QWidget* viewBase)
    : ui(viewBaseUI),
    mViewBase(viewBase),
    mTopParent(nullptr),
    mFadeOutWidget(nullptr),
    QObject(viewBase)
{
    qRegisterMetaType<MessageInfo>("MessageInfo");
    qRegisterMetaType<std::shared_ptr<MessageInfo>>("std::shared_ptr<MessageInfo>");

    mViewBase->installEventFilter(this);
}

LoadingSceneMessageHandler::~LoadingSceneMessageHandler()
{
    mFadeOutWidget->deleteLater();
}

void LoadingSceneMessageHandler::hideLoadingMessage()
{
    updateMessage(nullptr);
    sendLoadingMessageVisibilityChange(false);
    mLoadingViewVisible = false;
}

void LoadingSceneMessageHandler::setTopParent(QWidget *widget)
{
    if(!mTopParent)
    {
        mTopParent = widget;
        ui->wMessageContainer->setParent(mTopParent);
        mTopParent->installEventFilter(this);
    }
}

void LoadingSceneMessageHandler::updateMessage(std::shared_ptr<MessageInfo> info)
{
    if(!info)
    {
        sendLoadingMessageVisibilityChange(false);
        mCurrentInfo.reset();
    }
    else
    {
        if(!mLoadingViewVisible)
        {
            mCurrentInfo = info;
            return;
        }

        if(!info->message.isEmpty() && !ui->lMessageLabel->isVisible())
        {
            sendLoadingMessageVisibilityChange(true);
        }

        ui->lMessageLabel->setText(info->message);

        ui->pbProgressBar->setVisible(info->total != 0);
        ui->lProgressLabel->setVisible(info->total != 0);

        if(info->total != 0)
        {
            ui->lProgressLabel->setText(tr("%1 of %2").arg(info->count).arg(info->total));
            ui->pbProgressBar->setMaximum(info->total);
            ui->pbProgressBar->setValue(info->count);
        }

        if(info->buttonType != MessageInfo::ButtonType::None)
        {
            if(info->buttonType == MessageInfo::ButtonType::Stop)
            {
                ui->bStopButton->setVisible(info->total > 1);
                ui->bStopButton->setText(tr("Stop"));
            }
            else if(info->buttonType == MessageInfo::ButtonType::Ok)
            {
                ui->bStopButton->setVisible(true);
                ui->bStopButton->setText(tr("Ok"));
            }
        }
        else
        {
            ui->bStopButton->hide();
        }

        ui->wMessageContainer->adjustSize();
        updateMessagePos();
    }
}

bool LoadingSceneMessageHandler::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        updateMessagePos();
    }
    else if(event->type() == QEvent::KeyRelease && ui->wMessageContainer->isVisible())
    {
        auto keyEvent = dynamic_cast<QKeyEvent*>(event);
        if(keyEvent && keyEvent->key() == Qt::Key_Return)
        {
            ui->bStopButton->click();
        }
    }


    return QObject::eventFilter(watched, event);
}

void LoadingSceneMessageHandler::sendLoadingMessageVisibilityChange(bool value)
{
    if(value && !mFadeOutWidget)
    {
        mFadeOutWidget = new QWidget(mTopParent ? mTopParent : mViewBase);
        mFadeOutWidget->show();
        mFadeOutWidget->setStyleSheet(QString::fromLatin1("background-color: rgba(0,0,0,0.20);"));
    }

    if(mFadeOutWidget)
    {
        mFadeOutWidget->setVisible(value);
    }

    ui->wMessageContainer->setVisible(value);

    updateMessagePos();
    emit loadingMessageVisibilityChange(value);
}

void LoadingSceneMessageHandler::updateMessagePos()
{
    if(mFadeOutWidget)
    {
        if(mTopParent)
        {
            mFadeOutWidget->setGeometry(QRect(QPoint(0,0), mTopParent->size()));
        }
        else
        {
            mFadeOutWidget->resize(mViewBase->size());
            mFadeOutWidget->move(0,0);
        }

        mFadeOutWidget->stackUnder(ui->wMessageContainer);
    }
    else
    {
        ui->swLoadingViewContainer->stackUnder(ui->wMessageContainer);
    }

    auto messageGeo(ui->wMessageContainer->geometry());

    if(mTopParent)
    {
        QRect topRect(QPoint(0,0), mTopParent->size());
        messageGeo.moveCenter(topRect.center());
    }
    else
    {
        messageGeo.moveCenter(mViewBase->geometry().center());
    }

    ui->wMessageContainer->setGeometry(messageGeo);
    ui->wMessageContainer->raise();
}

void LoadingSceneMessageHandler::setLoadingViewVisible(bool newLoadingViewVisible)
{
    mLoadingViewVisible = newLoadingViewVisible;
    updateMessage(mCurrentInfo);
}
