#include "UsageWidget.h"

#include <QPainter>

UsageWidget::UsageWidget(QWidget *parent)
    : QWidget(parent)
{
    this->maxStorage.clear();

    overquotaReached = false;

    this->pCloud = 0;
    this->pRubbish = 0;
    this->pInbox = 0;
    this->pInShare = 0;

    cloudLabel.clear();
    rubbishLabel.clear();
    inShareLabel.clear();
    inboxLabel.clear();
    usedLabel.clear();
    availableLabel.clear();

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void UsageWidget::setCloudStorage(int percentage)
{
    this->pCloud = percentage;
    update();
}
void UsageWidget::setRubbishStorage(int percentage)
{
    this->pRubbish = percentage;
    update();
}
void UsageWidget::setInboxStorage(int percentage)
{
    this->pInbox = percentage;
    update();
}
void UsageWidget::setInShareStorage(int percentage)
{
    this->pInShare = percentage;
    update();
}
void UsageWidget::setCloudStorageLabel(QString amount)
{
    this->cloudLabel = amount;
    update();
}
void UsageWidget::setRubbishStorageLabel(QString amount)
{
    this->rubbishLabel = amount;
    update();
}
void UsageWidget::setInboxStorageLabel(QString amount)
{
    this->inboxLabel = amount;
    update();
}
void UsageWidget::setInShareStorageLabel(QString amount)
{
    this->inShareLabel = amount;
    update();
}
void UsageWidget::setUsedStorageLabel(QString amount)
{
    this->usedLabel = amount;
    update();
}
void UsageWidget::setAvailableStorageLabel(QString amount)
{
    this->availableLabel = overquotaReached ? QString::fromUtf8("0") : amount;
    update();
}

void UsageWidget::setMaxStorage(QString amount)
{
    this->maxStorage = amount;
    update();
}

void UsageWidget::setOverQuotaReached(bool value)
{
    this->overquotaReached = value;
    update();
}

QSize UsageWidget::minimumSizeHint() const
{
    return QSize(600, 136);
}
QSize UsageWidget::sizeHint() const
{
    return QSize(600, 136);
}

void UsageWidget::clearAll()
{
    maxStorage.clear();
    pCloud = 0;
    pRubbish = 0;
    pInbox = 0;
    pInShare = 0;

    cloudLabel.clear();
    rubbishLabel.clear();
    inShareLabel.clear();
    inboxLabel.clear();
    usedLabel.clear();
    availableLabel.clear();
}

void UsageWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QFont font;
    font.setPixelSize(20);
    font.setFamily(QString::fromUtf8("Helvetica"));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    painter.setFont(font);
    painter.translate(68,68);
    painter.setPen(QPen(QColor(0, 0, 0, 26), 2));
    painter.drawEllipse(QRectF(-88 / 2.0, -88 / 2.0, 88, 88));
    painter.setPen(QPen(QColor("#333333")));
    painter.drawText(QRectF(-17, -15, 35, 19),Qt::AlignCenter,QString::fromUtf8("%1").arg(maxStorage.isEmpty() ? QString::fromUtf8("") :maxStorage.split(QString::fromUtf8(" ")).at(0)));
    font.setPixelSize(12);
    painter.setFont(font);
    font.setWeight(QFont::Bold);
    painter.setPen(QPen(QColor("#666666")));
    painter.drawText(QRectF(-17,4, 35, 17),Qt::AlignCenter,QString::fromUtf8("%1").arg(maxStorage.isEmpty() ? QString::fromUtf8("") :maxStorage.split(QString::fromUtf8(" ")).at(1)));

    if (overquotaReached)
    {
        painter.setPen(QPen(QColor(Qt::red),10,Qt::SolidLine,Qt::FlatCap));
        painter.drawArc(QRectF(-72 / 2.0, -72 / 2.0, 72, 72), 90 * 16, -16 * 360);
    }
    else
    {
        painter.setPen(QPen(QColor(BLUE),10,Qt::SolidLine,Qt::FlatCap));
        painter.drawArc(QRectF(-72 / 2.0, -72 / 2.0, 72, 72), 90 * 16, -16 * pCloud);
        painter.setPen(QPen(QColor(GREEN),10,Qt::SolidLine,Qt::FlatCap));
        painter.drawArc(QRectF(-72 / 2.0, -72 / 2.0, 72, 72), 16 * (90 - pCloud), -16 * pRubbish);
        painter.setPen(QPen(QColor(YELLOW),10,Qt::SolidLine,Qt::FlatCap));
        painter.drawArc(QRectF(-72 / 2.0, -72 / 2.0, 72, 72), 16 * (90 - pCloud-pRubbish), -16 * pInShare);
        painter.setPen(QPen(QColor(ORANGE),10,Qt::SolidLine,Qt::FlatCap));
        painter.drawArc(QRectF(-72 / 2.0, -72 / 2.0, 72, 72), 16 * (90 - pCloud-pRubbish - pInShare), -16 * pInbox);
    }

    font.setPixelSize(13);
    font.setWeight(QFont::Normal);
    painter.setFont(font);

    painter.setPen(QPen(QColor(BLUE)));
    painter.setBrush(QBrush(QColor(BLUE)));
    painter.drawEllipse(QRectF(70, -35, 12, 12));
    painter.setPen(QPen(QColor("#666666")));
    painter.drawText(QRectF(90, -35, 130, 20),Qt::AlignLeft,tr("Cloud Drive"));
    painter.setPen(QPen(QColor("#333333")));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRectF(210, -35, 78, 20),Qt::AlignRight,QString::fromUtf8("%1").arg(cloudLabel));

    painter.setPen(QPen(QColor(GREEN)));
    painter.setBrush(QBrush(QColor(GREEN)));
    painter.drawEllipse(QRectF(70, -8, 12, 12));
    painter.setPen(QPen(QColor("#666666")));
    font.setWeight(QFont::Normal);
    painter.setFont(font);
    painter.drawText(QRectF(90, -8, 130, 20),Qt::AlignLeft,tr("Rubbish Bin"));
    painter.setPen(QPen(QColor("#333333")));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRectF(210, -8, 78, 20),Qt::AlignRight,QString::fromUtf8("%1").arg(rubbishLabel));

    painter.setPen(QPen(QColor(YELLOW)));
    painter.setBrush(QBrush(QColor(YELLOW)));
    painter.drawEllipse(QRectF(70, 20, 12, 12));
    painter.setPen(QPen(QColor("#666666")));
    font.setWeight(QFont::Normal);
    painter.setFont(font);
    painter.drawText(QRectF(90, 20, 130, 20),Qt::AlignLeft,tr("Incoming Shares"));
    painter.setPen(QPen(QColor("#333333")));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRectF(210, 20, 78, 20),Qt::AlignRight,QString::fromUtf8("%1").arg(inShareLabel));

    painter.setPen(QPen(QColor(ORANGE)));
    painter.setBrush(QBrush(QColor(ORANGE)));
    painter.drawEllipse(QRectF(313, -35, 12, 12));
    painter.setPen(QPen(QColor("#666666")));
    font.setWeight(QFont::Normal);
    painter.setFont(font);
    painter.drawText(QRectF(333, -35, 130, 20),Qt::AlignLeft,tr("Inbox"));
    painter.setPen(QPen(QColor("#333333")));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRectF(450, -35, 78, 20),Qt::AlignRight,QString::fromUtf8("%1").arg(inboxLabel));

    painter.setPen(QPen(QColor(GREY)));
    painter.setBrush(QBrush(QColor(GREY)));
    painter.drawEllipse(QRectF(313, -8, 12, 12));
    painter.setPen(QPen(QColor("#666666")));
    font.setWeight(QFont::Normal);
    painter.setFont(font);
    painter.drawText(QRectF(333, -8, 130,20),Qt::AlignLeft,tr("Used"));
    painter.setPen(QPen(QColor("#333333")));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRectF(450, -8, 78, 20),Qt::AlignRight,QString::fromUtf8("%1").arg(usedLabel));

    painter.setPen(QPen(QColor(0, 0, 0, 50), 2));
    painter.setBrush(QBrush(Qt::NoBrush));
    painter.drawEllipse(QRectF(313, 20, 12, 12));
    painter.setPen(QPen(QColor("#666666")));
    font.setWeight(QFont::Normal);
    painter.setFont(font);
    painter.drawText(QRectF(333, 20, 130, 20),Qt::AlignLeft,tr("Available"));
    painter.setPen(QPen(QColor("#333333")));
    font.setWeight(QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRectF(390, 20, 138, 20),Qt::AlignRight,QString::fromUtf8("%1").arg(availableLabel));
}
