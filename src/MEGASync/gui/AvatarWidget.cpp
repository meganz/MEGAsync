#include "AvatarWidget.h"
#include "control/Utilities.h"
#include <QPainter>
#include <QMouseEvent>
#include <math.h>
#include "MegaApplication.h"

AvatarWidget::AvatarWidget(QWidget *parent) :
    QWidget(parent)
{
    lastloadedwidth = 0;
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

    qreal factor = width() / 36.0;
    int backgroundUnscaledWidth = qFloor(36.0 * factor/2) * 2;
    int backgroundUnscaledHeight = qFloor(36.0 * factor/2) * 2;

    if (!lastloadedwidth || lastloadedwidth != width())
    {
        if (!lastloadedwidth)
        {
            backgroundPixmapOriginal = QIcon(QString::fromUtf8(":/images/avatar_frame.png")).pixmap(36.0, 36.0);
        }

    #if QT_VERSION >= 0x050000
        if (width() != 36.0 )
        {
            backgroundPixmap = backgroundPixmapOriginal.scaled(backgroundUnscaledWidth*qApp->devicePixelRatio(), backgroundUnscaledHeight*qApp->devicePixelRatio());
            backgroundPixmap.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
        }
        else
    #endif
        {
            backgroundPixmap = backgroundPixmapOriginal;
        }

        lastloadedwidth = width();
    }

    int innercirclediam = qFloor(24.0 * factor / 2) * 2;

    painter.translate(width() / 2, height() / 2);
    painter.drawPixmap(- qRound(backgroundUnscaledWidth / 2.0 ), - qRound(backgroundUnscaledHeight / 2.0), backgroundPixmap);

    if (QFileInfo(pathToFile).exists())
    {
        // Draw circular mask
        QImage imageMask(36.0 * factor, 36.0 * factor, QImage::Format_ARGB32_Premultiplied);
        imageMask.fill(Qt::transparent);
        QPainter mask(&imageMask);
        mask.setRenderHints(QPainter::Antialiasing
                        | QPainter::SmoothPixmapTransform
                        | QPainter::HighQualityAntialiasing);
        mask.setPen(Qt::NoPen);
        mask.setBrush(Qt::white);
        mask.drawEllipse(QRectF(0, 0, 36 * factor, 36 * factor));

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
        painter.drawPixmap(QRect(-innercirclediam / 2, -innercirclediam / 2 , innercirclediam, innercirclediam), QPixmap::fromImage(avatar));
    }
    else
    {
        QFont font;
        font.setPixelSize(18.0 * factor);
        font.setFamily(QString::fromUtf8("Lato"));
        painter.setFont(font);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(color.size() ? color : QString::fromUtf8("#D90007"))));
        painter.drawEllipse(QRect(-innercirclediam / 2, -innercirclediam / 2, innercirclediam, innercirclediam));

        painter.setPen(QPen(QColor("#ffffff")));
        painter.drawText(QRect(-innercirclediam / 2, -innercirclediam / 2, innercirclediam, innercirclediam), Qt::AlignCenter, letter);
    }
}

void AvatarWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        ((MegaApplication *)qApp)->openSettings(SettingsDialog::ACCOUNT_TAB);
    }
}

AvatarWidget::~AvatarWidget()
{

}
