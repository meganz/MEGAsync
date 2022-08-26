#include "StalledIssueActionTitle.h"
#include "ui_StalledIssueActionTitle.h"

#include <Utilities.h>

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QDebug>

const char* BUTTON_ID = "button_id";
const char* ONLY_ICON = "onlyIcon";
const char* MAIN_BUTTON = "main";
const char* DISCARDED = "discarded";

#include <QGraphicsOpacityEffect>

StalledIssueActionTitle::StalledIssueActionTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueActionTitle),
    mIsCloud(false)
{
    ui->setupUi(this);
    ui->icon->hide();

    ui->titleLabel->installEventFilter(this);
}

StalledIssueActionTitle::~StalledIssueActionTitle()
{
    delete ui;
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
    QFileInfo fileInfo(title());
    QIcon fileTypeIcon;

    if(!fileInfo.completeSuffix().isEmpty())
    {
        fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                      title(), QLatin1Literal(":/images/drag_")));
    }
    else
    {
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/folder_orange_default@2x.png"));
    }

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

void StalledIssueActionTitle::setSolved(bool state)
{
    ui->contents->setProperty(DISCARDED,state);
    setStyleSheet(styleSheet());

    if(state)
    {
        auto effect = new QGraphicsOpacityEffect(this);
        effect->setOpacity(0.30);
        ui->titleContainer->setGraphicsEffect(effect);
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


