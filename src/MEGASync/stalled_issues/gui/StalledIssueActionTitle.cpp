#include "StalledIssueActionTitle.h"
#include "ui_StalledIssueChooseTitle.h"

#include <Utilities.h>

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>

const char* BUTTON_ID = "button_id";
const char* ONLY_ICON = "onlyIcon";

StalledIssueActionTitle::StalledIssueActionTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueChooseTitle),
    mRoundedCorners(RoundedCorners::TOP_CORNERS)
{
    ui->setupUi(this);
    ui->titleLabel->installEventFilter(this);
    ui->icon->hide();
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

void StalledIssueActionTitle::addActionButton(const QIcon& icon,const QString &text, int id)
{
    auto button = new QPushButton(icon, text, this);

    button->setProperty(BUTTON_ID, id);
    button->setProperty(ONLY_ICON, false);
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
            ui->actionContainer->setStyleSheet(ui->actionContainer->styleSheet());
        }
    }
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

void StalledIssueActionTitle::addMessage(const QString &message)
{
    auto messageLabel = new QLabel(message);
    ui->actionLayout->addWidget(messageLabel);
}

void StalledIssueActionTitle::setIndent(int indent)
{
    auto chooseMargins = contentsMargins();
    chooseMargins.setLeft(indent);
    setContentsMargins(chooseMargins);
}

void StalledIssueActionTitle::setRoundedCorners(RoundedCorners type)
{
    mRoundedCorners = type;
    update();
}

void StalledIssueActionTitle::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QColor backgroundColor = palette().color(isEnabled() ? QPalette::ColorGroup::Active : QPalette::ColorGroup::Disabled, QPalette::ColorRole::Background);
    painter.setBrush(backgroundColor);
    painter.setPen(Qt::NoPen);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    if(mRoundedCorners == RoundedCorners::ALL_CORNERS)
    {
        path.addRoundedRect( QRect(0,0, width(), height()), 6, 6);
    }
    else if(mRoundedCorners == RoundedCorners::TOP_CORNERS)
    {
        path.addRoundedRect( QRect(0,0, width(), 6), 6, 6);
        path.addRect(QRect( 0, 3, width(), height() -3)); // Top right corner not rounded
    }

    painter.drawPath(path.simplified());
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


