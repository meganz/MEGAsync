#include "ViewLoadingMessage.h"

#include "BlurredShadowEffect.h"
#include "ui_ViewLoadingMessage.h"
#include "ViewLoadingScene.h"

ViewLoadingMessage::ViewLoadingMessage(QWidget* parent):
    QWidget(parent),
    ui(new Ui::ViewLoadingMessage),
    mCloseWhenAnyButtonIsPressed(false),
    mWaitingForAnswer(false)
{
    ui->setupUi(this);

    parent->installEventFilter(this);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->bButton, &QPushButton::clicked, this, &ViewLoadingMessage::onButtonPressed);

    setProperty("TOKENIZED", true);

    // Shadow
    setGraphicsEffect(CreateBlurredShadowEffect(QColor(0, 0, 0, 90), 30, 0, 25));
}

ViewLoadingMessage::~ViewLoadingMessage()
{
    delete ui;
}

void ViewLoadingMessage::onButtonPressed()
{
    ui->bButton->setDisabled(true);
    mWaitingForAnswer = false;
    emit buttonPressed(getButtonType());

    // The message is not longer needed, as the user has closed it
    // If the button type is stop, normally we update the ui and we then show the OK button
    // If mCloseWhenAnyButtonIsPressed is set, the message is closed regardless the button pressed
    if (mCloseWhenAnyButtonIsPressed || mInfo->buttonType == MessageInfo::ButtonType::OK)
    {
        close();
    }
}

void ViewLoadingMessage::setCloseWhenAnyButtonIsPressed(bool newCloseWhenAnyButtonIsPressed)
{
    mCloseWhenAnyButtonIsPressed = newCloseWhenAnyButtonIsPressed;
}

void ViewLoadingMessage::updateMessage(std::shared_ptr<MessageInfo> info)
{
    if (info)
    {
        mInfo = info;

        ui->lMessageLabel->setText(info->message);

        ui->pbProgressBar->setVisible(info->total != 0);
        ui->lProgressLabel->setVisible(info->total != 0);

        if (info->total != 0)
        {
            ui->lProgressLabel->setText(
                QApplication::translate("LoadingSceneMessageHandler", "%1 of %2")
                    .arg(info->count)
                    .arg(info->total));
            ui->pbProgressBar->setMaximum(info->total);
            ui->pbProgressBar->setValue(info->count);
        }

        if (info->buttonType != MessageInfo::ButtonType::NONE)
        {
            ui->bButton->setDisabled(false);

            if (info->buttonType == MessageInfo::ButtonType::STOP)
            {
                ui->bButton->setVisible(info->total > 1);
                ui->bButton->setText(QApplication::translate("LoadingSceneMessageHandler", "Stop"));
            }
            else if (info->buttonType == MessageInfo::ButtonType::OK)
            {
                ui->bButton->setVisible(true);
                ui->bButton->setText(QApplication::translate("LoadingSceneMessageHandler", "Ok"));
            }

            mWaitingForAnswer = true;
        }
        else
        {
            ui->bButton->hide();
            ui->bButton->setText(QString());

            mWaitingForAnswer = false;
        }

        ui->wMessageContainer->adjustSize();
        updateGeometry();
    }
}

void ViewLoadingMessage::updateGeometry()
{
    setGeometry(QRect(QPoint(0, 0), parentWidget()->size()));
    raise();
}

bool ViewLoadingMessage::isWaitingForAnswer() const
{
    return mWaitingForAnswer;
}

int ViewLoadingMessage::getButtonType() const
{
    return mInfo->buttonType;
}

bool ViewLoadingMessage::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parent() && event->type() == QEvent::Resize)
    {
        updateGeometry();
    }

    return QWidget::eventFilter(watched, event);
}
