#include <ViewLoadingScene.h>
#include "ui_ViewLoadingScene.h"

#include <QDebug>
#include <QKeyEvent>


ViewLoadingSceneBase::ViewLoadingSceneBase() :
    mDelayTimeToShowInMs(0),
    mLoadingView(nullptr),
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
    ui->MessageContainer->hide();

    connect(ui->StopButton, &QPushButton::clicked, mMessageHandler, &LoadingSceneMessageHandler::onStopPressed);
}

void ViewLoadingSceneBase::show()
{
    ui->MessageContainer->hide();
    mLoadingSceneUI->show();
    mLoadingView->show();
    ui->LoadingViewContainer->stackUnder(ui->MessageContainer);
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
        ui->LoadingViewContainer->resize(mLoadingSceneUI->size());
        ui->LoadingViewContainer->move(0,0);
    }

    return QObject::eventFilter(watched, event);
}

LoadingSceneMessageHandler::LoadingSceneMessageHandler(Ui::ViewLoadingSceneUI *viewBaseUI, QWidget* viewBase)
    : ui(viewBaseUI),
      mViewBase(viewBase),
      mTopParent(nullptr),
      mFadeOutWidget(nullptr),
      QObject(viewBase)
{
    qRegisterMetaType<MessageInfo>("MessageInfo");

    mViewBase->installEventFilter(this);
}

LoadingSceneMessageHandler::~LoadingSceneMessageHandler()
{
    mFadeOutWidget->deleteLater();
}

void LoadingSceneMessageHandler::hideLoadingMessage()
{
    updateMessage(MessageInfo());
    sendLoadingMessageVisibilityChange(false);
}

void LoadingSceneMessageHandler::setTopParent(QWidget *widget)
{
    mTopParent = widget;
    ui->MessageContainer->setParent(mTopParent);
    mTopParent->installEventFilter(this);
}

void LoadingSceneMessageHandler::updateMessage(const MessageInfo &info)
{
    if(info.message.isEmpty() && ui->MessageContainer->isVisible())
    {
        sendLoadingMessageVisibilityChange(false);
    }
    else
    {
        if(!info.message.isEmpty() && !ui->MessageLabel->isVisible())
        {
            sendLoadingMessageVisibilityChange(true);
        }

        ui->MessageLabel->setText(info.message);

        ui->ProgressBar->setVisible(info.total != 0);
        ui->ProgressLabel->setVisible(info.total != 0);

        if(info.total != 0)
        {
            ui->ProgressLabel->setText(tr("%1 of %2").arg(info.count).arg(info.total));
            ui->ProgressBar->setMaximum(info.total);
            ui->ProgressBar->setValue(info.count);
        }

        ui->StopButton->setVisible(info.total > 1 || info.buttonType == MessageInfo::ButtonType::Ok);
        ui->StopButton->setText(info.buttonType == MessageInfo::ButtonType::Stop ? tr("Stop") : tr("Ok"));

        ui->MessageContainer->adjustSize();
    }
}

bool LoadingSceneMessageHandler::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        updateMessagePos();
    }
    else if(event->type() == QEvent::KeyRelease && ui->MessageContainer->isVisible())
    {
        auto keyEvent = dynamic_cast<QKeyEvent*>(event);
        if(keyEvent && keyEvent->key() == Qt::Key_Return)
        {
            ui->StopButton->click();
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

    ui->MessageContainer->setVisible(value);

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

        mFadeOutWidget->stackUnder(ui->MessageContainer);
    }
    else
    {
        ui->LoadingViewContainer->stackUnder(ui->MessageContainer);
    }

    auto messageGeo(ui->MessageContainer->geometry());
    if(mTopParent)
    {
        QRect topRect(QPoint(0,0), mTopParent->size());
        messageGeo.moveCenter(topRect.center());
    }
    else
    {
        messageGeo.moveCenter(mViewBase->geometry().center());
    }
    ui->MessageContainer->setGeometry(messageGeo);
    ui->MessageContainer->raise();
}
