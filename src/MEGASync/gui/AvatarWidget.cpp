#include "AvatarWidget.h"
#include "control/Utilities.h"
#include <QPainter>
#include <QBitmap>
#include <math.h>

AvatarWidget::AvatarWidget(QWidget *parent) :
    QWidget(parent)
{
    clearData();
}

void AvatarWidget::setAvatarLetter(QChar letter, const char* color)
{
    this->letter = letter;
    if (color)
    {
        this->color = color;
    }
    update();
}

void AvatarWidget::setAvatarImage(QString pathToFile)
{
    this->pathToFile = pathToFile;
    update();
}

void AvatarWidget::clearData()
{
    letter = QChar();
    pathToFile = QString();
    color = NULL;
}

QSize AvatarWidget::minimumSizeHint() const
{
    return QSize(30, 30);
}

QSize AvatarWidget::sizeHint() const
{
    return QSize(30, 30);
}

void AvatarWidget::paintEvent(QPaintEvent *event)
{
    if (letter.isNull() && pathToFile.isNull())
    {
        return;
    }

    int radius = ceil(width() - width() * 0.20) / 2;
    QPainter painter(this);

    // Draw border
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    painter.setPen(QPen(QColor(0, 0, 0, 26), 1));
    painter.drawEllipse(QRectF(1, 1, width() - 2 , height() - 2));
    painter.translate(width() / 2, height() / 2);

    if (QFileInfo(pathToFile).exists())
    {
        // Draw circular mask
        QImage imageMask(width(), height(), QImage::Format_ARGB32_Premultiplied);
        imageMask.fill(Qt::transparent);
        QPainter mask(&imageMask);
        mask.setRenderHints(QPainter::Antialiasing
                        | QPainter::SmoothPixmapTransform
                        | QPainter::HighQualityAntialiasing);
        mask.setPen(Qt::NoPen);
        mask.setBrush(Qt::white);
        mask.drawEllipse(QRectF(0, 0, width(), height()));

        // Composite mask and avatar
        QImage avatar(imageMask.size(), imageMask.format());
        QImage img(pathToFile);
        QPainter p(&avatar);
        p.setRenderHints(QPainter::Antialiasing
                        | QPainter::SmoothPixmapTransform
                        | QPainter::HighQualityAntialiasing);
        p.drawImage(QRect(0, 0, imageMask.width(), imageMask.height()), img);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawImage(0, 0, imageMask);

        //Apply avatar
        painter.drawPixmap(QRect(-radius, -radius, radius * 2, radius * 2), QPixmap::fromImage(avatar));
    }
    else
    {
        QFont font;
        font.setPixelSize(18);
        font.setFamily(QString::fromUtf8("Source Sans Pro"));
        painter.setFont(font);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(color ? color : "#D90007")));
        painter.drawEllipse(QRect(-radius, -radius, radius * 2, radius * 2));
        painter.setPen(QPen(QColor("#ffffff")));
        painter.drawText(QRect(-radius, -radius, radius * 2, radius * 2), Qt::AlignCenter, letter);
    }
}

AvatarWidget::~AvatarWidget()
{
    delete color;
}
