#include "ui_ViewLoadingScene.h"
#include "ViewLoadingMessage.h"
#include <ViewLoadingScene.h>

#include <QKeyEvent>

#include <memory>

ViewLoadingSceneBase::ViewLoadingSceneBase() :
      mDelayTimeToShowInMs(0)
    , mLoadingView(nullptr)
    , ui(new Ui::ViewLoadingSceneUI())
    , mTopParent(nullptr)
    , mLoadingViewSet(LoadingViewType::NONE)
    , mMessageHandler(nullptr)
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
    mMessageHandler->setLoadingViewVisible(false);
}

void ViewLoadingSceneBase::showLoadingScene()
{
    ui->wParentViewCopy->hide();
    mMessageHandler->setLoadingViewVisible(true);
}

LoadingSceneMessageHandler::LoadingSceneMessageHandler(Ui::ViewLoadingSceneUI* viewBaseUI,
                                                       QWidget* viewBase):
    QObject(viewBase),
    ui(viewBaseUI),
    mViewBase(viewBase),
    mTopParent(nullptr),
    mLoadingMessage(nullptr)
{
    mViewBase->installEventFilter(this);

    qRegisterMetaType<MessageInfo>("MessageInfo");
    qRegisterMetaType<std::shared_ptr<MessageInfo>>("std::shared_ptr<MessageInfo>");
}

LoadingSceneMessageHandler::~LoadingSceneMessageHandler()
{
    mLoadingMessage->deleteLater();
}

bool LoadingSceneMessageHandler::needsAnswerFromUser() const
{
    return mLoadingMessage && mLoadingMessage->isButtonVisible();
}

void LoadingSceneMessageHandler::hideLoadingMessage()
{
    updateMessage(nullptr);
    mLoadingViewVisible = false;
    checkLoadingMessageVisibility();
}

void LoadingSceneMessageHandler::setTopParent(QWidget *widget)
{
    if(!mTopParent)
    {
        mTopParent = widget;
        mTopParent->installEventFilter(this);
    }
}

void LoadingSceneMessageHandler::updateMessage(std::shared_ptr<MessageInfo> info)
{
    if (info)
    {
        createLoadingMessage();
        mLoadingMessage->updateMessage(info);
        checkLoadingMessageVisibility();
    }
}

bool LoadingSceneMessageHandler::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease && mLoadingMessage)
    {
        auto keyEvent = dynamic_cast<QKeyEvent*>(event);
        if(keyEvent && keyEvent->key() == Qt::Key_Return)
        {
            onButtonPressed(mLoadingMessage->getButtonType());
        }
    }

    return QObject::eventFilter(watched, event);
}

void LoadingSceneMessageHandler::onButtonPressed(int buttonType)
{
    emit buttonPressed(static_cast<MessageInfo::ButtonType>(buttonType));
}

void LoadingSceneMessageHandler::checkLoadingMessageVisibility()
{
    if (mLoadingMessage)
    {
        if (mLoadingViewVisible)
        {
            mLoadingMessage->updateGeometry();
        }

        if (mLoadingViewVisible)
        {
            mLoadingMessage->setVisible(true);
        }
        else
        {
            mLoadingMessage->close();
        }
    }
}

void LoadingSceneMessageHandler::createLoadingMessage()
{
    if (!mLoadingMessage)
    {
        mLoadingMessage = new ViewLoadingMessage(mTopParent ? mTopParent : mViewBase);
        mLoadingMessage->hide();
        connect(mLoadingMessage,
                &ViewLoadingMessage::buttonPressed,
                this,
                &LoadingSceneMessageHandler::onButtonPressed);
    }
}

void LoadingSceneMessageHandler::setLoadingViewVisible(bool newLoadingViewVisible)
{
    mLoadingViewVisible = newLoadingViewVisible;
    checkLoadingMessageVisibility();
}
