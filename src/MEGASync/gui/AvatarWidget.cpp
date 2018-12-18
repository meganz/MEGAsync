#include "AvatarWidget.h"
#include "control/Utilities.h"
#include <QPainter>
#include <math.h>

AvatarWidget::AvatarWidget(QWidget *parent) :
    QWidget(parent)
{
    clearData();
}

void AvatarWidget::setAvatarLetter(QChar letter, QString color)
{
    this->letter = letter;
    this->color = color;
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
    color = QString();
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

    QPainter painter(this);

    // Draw border image
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    QPixmap buffer;
#if QT_VERSION >= 0x050000
    buffer.setDevicePixelRatio(Utilities::getDevicePixelRatio());
#endif
    buffer.load(QString::fromUtf8(":/images/avatar_frame.png"));

    painter.drawPixmap(QRectF(0.0, 0.0, 36.0, 36.0), buffer,  QRectF(0.0, 0.0, 36.0, 36.0));
    painter.translate(width() / 2, height() / 2);

    if (QFileInfo(pathToFile).exists())
    {
        // Draw circular mask
        QImage imageMask(36.0, 36.0, QImage::Format_ARGB32_Premultiplied);
        imageMask.fill(Qt::transparent);
        QPainter mask(&imageMask);
        mask.setRenderHints(QPainter::Antialiasing
                        | QPainter::SmoothPixmapTransform
                        | QPainter::HighQualityAntialiasing);
        mask.setPen(Qt::NoPen);
        mask.setBrush(Qt::white);
        mask.drawEllipse(QRectF(0, 0, 36, 36));

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
        painter.drawPixmap(QRect(-12, -12, 24, 24), QPixmap::fromImage(avatar));
    }
    else
    {
        QFont font;
        font.setPixelSize(18);
        font.setFamily(QString::fromUtf8("Source Sans Pro"));
        painter.setFont(font);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(color.size() ? color : QString::fromUtf8("#D90007"))));
        painter.drawEllipse(QRect(-12, -12, 24, 24));
        painter.setPen(QPen(QColor("#ffffff")));
        painter.drawText(QRect(-12, -12, 24, 24), Qt::AlignCenter, letter);
    }
}

AvatarWidget::~AvatarWidget()
{
}
