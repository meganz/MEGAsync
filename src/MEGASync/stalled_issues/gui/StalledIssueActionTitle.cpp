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

#include <QGraphicsOpacityEffect>

StalledIssueActionTitle::StalledIssueActionTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueActionTitle),
    mIsCloud(false)
{
    ui->setupUi(this);
    ui->icon->hide();

    ui->titleLabel->installEventFilter(this);

    ui->extraInfoContainer->hide();
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
    mTitle = title;
    ui->titleLabel->setText(mTitle);
}

QString StalledIssueActionTitle::title() const
{
    return mTitle;
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
}

void StalledIssueActionTitle::hideActionButton(int id)
{
    auto buttons = ui->actionContainer->findChildren<QPushButton*>();
    foreach(auto& button, buttons)
    {
        if(button->property(BUTTON_ID).toInt() == id)
        {
            button->hide();
        }
    }
}

void StalledIssueActionTitle::showIcon()
{
    QFileInfo fileInfo(mPath);
    QIcon fileTypeIcon = StalledIssuesUtilities::getFileIcon(fileInfo, false);

    ui->icon->setPixmap(fileTypeIcon.pixmap(ui->icon->size()));
    ui->icon->show();
}

void StalledIssueActionTitle::addMessage(const QString &message, const QPixmap& pixmap)
{
    QWidget* labelContainer= new QWidget(this);
    QHBoxLayout* labelContainerLayout = new QHBoxLayout();
    labelContainerLayout->setContentsMargins(0,0,10,0);
    labelContainer->setLayout(labelContainerLayout);

    if(!pixmap.isNull())
    {
        auto iconLabel = new QLabel(this);
        iconLabel->setPixmap(pixmap);
        labelContainerLayout->addWidget(iconLabel);
    }

    auto messageLabel = new QLabel(message,this);
    messageLabel->setText(message);
    labelContainerLayout->addWidget(messageLabel);

    ui->actionLayout->addWidget(labelContainer);
}

QLabel* StalledIssueActionTitle::addExtraInfo(const QString &title, const QString &info, int level)
{
    ui->extraInfoContainer->show();

    QWidget* container(new QWidget(this));
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

    auto& items = mExtraInfoItemsByRow[level];

    if(items != 0)
    {
        ui->extraInfoLayout->setColumnStretch(items, 0);
    }

    ui->extraInfoLayout->addWidget(container, level, items, Qt::AlignLeft);
    items++;

    ui->extraInfoLayout->setColumnStretch(items, 1);

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
    if(!mTitle.isEmpty() && watched == ui->titleLabel && event->type() == QEvent::Resize)
    {
        auto elidedText = ui->titleLabel->fontMetrics().elidedText(mTitle,Qt::ElideMiddle, ui->titleLabel->width());
        ui->titleLabel->setText(elidedText);
    }

    return QWidget::eventFilter(watched, event);
}

void StalledIssueActionTitle::updateUser(const QString &user)
{
    if(!mUserLabel)
    {
        mUserLabel = addExtraInfo(tr("Upload by:"), user, 1);
    }
    else
    {
        mUserLabel->setText(user);
    }
}

void StalledIssueActionTitle::updateSize(const QString &size)
{
    if(!mSizeLabel)
    {
        mSizeLabel = addExtraInfo(tr("Size:"), size, 0);
    }
    else
    {
        mSizeLabel->setText(size);
    }
}

void StalledIssueActionTitle::setPath(const QString &newPath)
{
    mPath = newPath;
}

void StalledIssueActionTitle::updateLastTimeModified(const QDateTime& time)
{
    auto timeString = MegaSyncApp->getFormattedDateByCurrentLanguage(time, QLocale::FormatType::ShortFormat);
    if(!mLastTimeLabel)
    {
        mLastTimeLabel = addExtraInfo(tr("Last modified:"), timeString, 0);
    }
    else
    {
        mLastTimeLabel->setText(timeString);
    }

}

void StalledIssueActionTitle::updateCreatedTime(const QDateTime& time)
{
    auto timeString = MegaSyncApp->getFormattedDateByCurrentLanguage(time, QLocale::FormatType::ShortFormat);
    if(!mCreatedTimeLabel)
    {
        mCreatedTimeLabel = addExtraInfo(mIsCloud ? tr("Upload at:") : tr("Created at:"), timeString, 0);
    }
    else
    {
        mCreatedTimeLabel->setText(timeString);
    }
}
