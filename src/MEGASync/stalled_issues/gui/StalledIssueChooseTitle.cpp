#include "StalledIssueChooseTitle.h"
#include "ui_StalledIssueChooseTitle.h"

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>

const char* BUTTON_ID = "BUTTON_ID";

StalledIssueActionTitle::StalledIssueActionTitle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StalledIssueChooseTitle)
{
    ui->setupUi(this);
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

void StalledIssueActionTitle::addActionButton(const QString &text, int id)
{
    auto button = new QPushButton(text, this);
    button->setProperty(BUTTON_ID, id);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked, this, [this]()
    {
       emit actionClicked(sender()->property(BUTTON_ID).toInt());
    });

    ui->actionLayout->addWidget(button);
}

void StalledIssueActionTitle::setIndent(int indent)
{
    auto chooseMargins = contentsMargins();
    chooseMargins.setLeft(indent);
    setContentsMargins(chooseMargins);
}

void StalledIssueActionTitle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(QColor("#F5F5F5"));
    painter.setPen(Qt::NoPen);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect( QRect(0,0, width(), 6), 6, 6);
    path.addRect(QRect( 0, 3, width(), height() -3)); // Top right corner not rounded
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


