#include "StalledIssueActionTitle.h"
#include "ui_StalledIssueActionTitle.h"

#include <Utilities.h>
#include <MegaApplication.h>
#include <StalledIssuesUtilities.h>

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QHBoxLayout>

const char* BUTTON_ID = "button_id";
const char* ONLY_ICON = "onlyIcon";
const char* MAIN_BUTTON = "main";
const char* DISCARDED = "discarded";
const char* TITLE = "title";
const char* DISABLE_BACKGROUND = "disable_background";
const char* MESSAGE_TEXT = "message_text";

#include <QGraphicsOpacityEffect>

StalledIssueActionTitle::StalledIssueActionTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueActionTitle),
    mIsCloud(false)
{
    ui->setupUi(this);

    ui->icon->hide();
    ui->messageContainer->hide();
    ui->actionContainer->hide();
    ui->extraInfoContainer->hide();

    ui->titleLabel->installEventFilter(this);

    ui->backgroundWidget->setProperty(DISABLE_BACKGROUND, false);
}

StalledIssueActionTitle::~StalledIssueActionTitle()
{
    delete ui;
}

void StalledIssueActionTitle::removeBackgroundColor()
{
    ui->backgroundWidget->setProperty(DISABLE_BACKGROUND, true);
}

void StalledIssueActionTitle::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
    ui->titleLabel->setProperty(MESSAGE_TEXT, title);
}

QString StalledIssueActionTitle::title() const
{
    return ui->titleLabel->property(MESSAGE_TEXT).toString();
}

void StalledIssueActionTitle::addActionButton(const QIcon& icon,const QString &text, int id, bool mainButton)
{
    auto button = new QPushButton(icon, text, this);

    button->setProperty(BUTTON_ID, id);
    button->setProperty(ONLY_ICON, false);
    button->setProperty(MAIN_BUTTON,mainButton);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked, this, [this]()
    {
       emit actionClicked(sender()->property(BUTTON_ID).toInt());
    });

    ui->actionLayout->addWidget(button);

    if(!icon.isNull())
    {
        button->setIconSize(QSize(24,24));

        if(text.isEmpty())
        {
            button->setProperty(ONLY_ICON, true);
        }
    }

    ui->actionContainer->setStyleSheet(ui->actionContainer->styleSheet());
    ui->actionContainer->show();
}

void StalledIssueActionTitle::hideActionButton(int id)
{
    bool allHidden(true);

    auto buttons = ui->actionContainer->findChildren<QPushButton*>();
    foreach(auto& button, buttons)
    {
        if(button->property(BUTTON_ID).toInt() == id)
        {
            button->hide();
        }
        else if(button->isVisible())
        {
            allHidden = false;
        }
    }

    if(allHidden)
    {
        ui->actionContainer->hide();
    }
}

void StalledIssueActionTitle::showIcon()
{
    QIcon fileTypeIcon;
    QFileInfo fileInfo(mPath);

    if(mIsCloud)
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(mPath.toUtf8().constData()));
        fileTypeIcon = StalledIssuesUtilities::getRemoteFileIcon(node.get(), fileInfo, false);
    }
    else
    {
        fileTypeIcon = StalledIssuesUtilities::getLocalFileIcon(fileInfo, false);
    }

    ui->icon->setPixmap(fileTypeIcon.pixmap(ui->icon->size()));
    ui->icon->show();
}

void StalledIssueActionTitle::addMessage(const QString &message, const QPixmap& pixmap)
{
    QWidget* labelContainer= new QWidget(this);
    labelContainer->installEventFilter(this);
    labelContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout* labelContainerLayout = new QHBoxLayout();
    labelContainerLayout->setContentsMargins(0,0,10,0);
    labelContainer->setLayout(labelContainerLayout);
    labelContainerLayout->addStretch();

    if(!pixmap.isNull())
    {
        auto iconLabel = new QLabel(this);
        iconLabel->setPixmap(pixmap);
        labelContainerLayout->addWidget(iconLabel);
    }

    auto messageLabel = new QLabel(message,this);
    messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    messageLabel->setText(message);
    messageLabel->setProperty(MESSAGE_TEXT, message);
    labelContainerLayout->addWidget(messageLabel);

    ui->messageContainer->show();
    ui->messageLayout->addWidget(labelContainer);
}

QLabel* StalledIssueActionTitle::addExtraInfo(const QString &title, const QString &info, int level)
{
    ui->extraInfoContainer->show();

    QWidget* container(new QWidget());
    QHBoxLayout* layout(new QHBoxLayout(container));
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(3);
    container->setLayout(layout);

    auto titleLabel = new QLabel(title, this);
    titleLabel->setProperty(TITLE, true);
    auto infoLabel = new QLabel(info, this);
    infoLabel->setProperty(TITLE, false);

    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);

    QHBoxLayout* rowLayout(nullptr);
    auto rowItem = ui->extraInfoLayout->itemAt(level);
    if(!rowItem)
    {
        auto rowWidget = new QWidget();
        rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0,0,0,0);
        rowLayout->setSpacing(20);
        rowLayout->addStretch();

        ui->extraInfoLayout->addWidget(rowWidget);
    }
    else
    {
        rowLayout = dynamic_cast<QHBoxLayout*>(rowItem->widget()->layout());
    }

    rowLayout->insertWidget(rowLayout->count()-1, container, 0, Qt::AlignLeft);

    setStyleSheet(styleSheet());

    return infoLabel;
}

void StalledIssueActionTitle::setDisabled(bool state)
{
    ui->backgroundWidget->setProperty(DISCARDED,state);
    setStyleSheet(styleSheet());

    if(state && !ui->titleContainer->graphicsEffect())
    {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.30);
        ui->titleContainer->setGraphicsEffect(effect);
    }

    if(state && !ui->extraInfoContainer->graphicsEffect())
    {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.30);
        ui->extraInfoContainer->setGraphicsEffect(effect);
    }
}

void StalledIssueActionTitle::setIsCloud(bool state)
{
    mIsCloud = state;
}

bool StalledIssueActionTitle::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::Resize)
    {
        if(watched == ui->titleLabel)
        {
            auto titleText = ui->titleLabel->property(MESSAGE_TEXT).toString();
            if(!titleText.isEmpty())
            {
                auto elidedText = ui->titleLabel->fontMetrics().elidedText(titleText, Qt::ElideMiddle, ui->titleLabel->width());
                ui->titleLabel->setText(elidedText);
            }
        }
        else
        {
            auto contentWidget = dynamic_cast<QWidget*>(watched);
            if(contentWidget)
            {
                auto childLabels = contentWidget->findChildren<QLabel*>();
                foreach(auto label, childLabels)
                {
                    if(label != ui->titleLabel)
                    {
                        auto elidedText = label->fontMetrics().elidedText(label->property(MESSAGE_TEXT).toString(), Qt::ElideMiddle, contentWidget->width() - 50);
                        label->setText(elidedText);
                    }
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueActionTitle::updateUser(const QString &user, bool show)
{
    if(show)
    {
        auto userText = user.isEmpty() ? tr("Loading user…") : user;
        auto& userLabel = mUpdateLabels[AttributeType::User];
        if(!userLabel)
        {
            userLabel = addExtraInfo(tr("Upload by:"), userText, 1);
        }
        else
        {
            userLabel->setText(userText);
        }

        showAttribute(AttributeType::User);
    }
    else
    {
        hideAttribute(AttributeType::User);
    }
}

void StalledIssueActionTitle::updateVersionsCount(int versions)
{
    if(versions > 1)
    {
        QString versionsText(QString::number(versions));
        auto& versionsLabel = mUpdateLabels[AttributeType::Versions];
        if(!versionsLabel)
        {
            versionsLabel = addExtraInfo(tr("Versions:"), versionsText, 1);
        }
        else
        {
            versionsLabel->setText(versionsText);
        }

        showAttribute(AttributeType::Versions);
    }
    else
    {
        hideAttribute(AttributeType::Versions);
    }
}

void StalledIssueActionTitle::updateSize(const QString &size)
{
    auto sizeText = size.isEmpty() ? tr("Loading size…") : size;

    auto& sizeLabel = mUpdateLabels[AttributeType::Size];
    if(!sizeLabel)
    {
        sizeLabel = addExtraInfo(tr("Size:"), sizeText, 0);
    }
    else
    {
        sizeLabel->setText(sizeText);
    }

    showAttribute(AttributeType::Size);
}

void StalledIssueActionTitle::updateLastTimeModified(const QDateTime& time)
{
    auto timeString = time.isValid() ? MegaSyncApp->getFormattedDateByCurrentLanguage(time, QLocale::FormatType::ShortFormat) : tr("Loading time…");
    auto& lastTimeLabel = mUpdateLabels[AttributeType::LastModified];
    if(!lastTimeLabel)
    {
        lastTimeLabel = addExtraInfo(tr("Last modified:"), timeString, 0);
    }
    else
    {
        lastTimeLabel->setText(timeString);
    }

    showAttribute(AttributeType::LastModified);
}

void StalledIssueActionTitle::updateCreatedTime(const QDateTime& time)
{
    auto timeString = time.isValid() ? MegaSyncApp->getFormattedDateByCurrentLanguage(time, QLocale::FormatType::ShortFormat) : tr("Loading time…");
    auto& createdTimeLabel = mUpdateLabels[AttributeType::CreatedTime];
    if(!createdTimeLabel)
    {
        createdTimeLabel = addExtraInfo(mIsCloud ? tr("Upload at:") : tr("Created at:"),  timeString, 0);
    }
    else
    {
        createdTimeLabel->setText(timeString);
    }

    showAttribute(AttributeType::CreatedTime);
}

void StalledIssueActionTitle::hideAttribute(AttributeType type)
{
    auto updateLabel = mUpdateLabels.value(type);
    if(updateLabel)
    {
        updateLabel->parentWidget()->hide();
    }
}

void StalledIssueActionTitle::showAttribute(AttributeType type)
{
    auto updateLabel = mUpdateLabels.value(type);
    if(updateLabel)
    {
        updateLabel->parentWidget()->show();
    }
}

void StalledIssueActionTitle::setPath(const QString &newPath)
{
    mPath = newPath;
}
