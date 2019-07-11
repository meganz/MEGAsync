#include "TransfersSummaryWidget.h"
#include "ui_TransfersSummaryWidget.h"

#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QPainterPath>
#include <QTimer>

TransfersSummaryWidget::TransfersSummaryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersSummaryWidget)
{
    ui->setupUi(this);
    displacement = 0;
    status = Status::EXPANDED;
    originalwidth = -1;

    animationTimeMS = 0.8*1000;
    acceleration = 0.35;
    calculateSpeed();

    fontUploads.setFamily(QString::fromUtf8("Lato"));
    fontDownloads.setFamily(QString::fromUtf8("Lato"));

    brushgreyspeeds = QBrush(QColor(QString::fromUtf8("#E5E5E5")));
    brushwhitebackground = QBrush(QColor(QString::fromUtf8("#FFFFFF")));
    pentext = QPen(QColor("#333333"));

    completedDownloads = 0;
    completedUploads = 0;
    totalUploads = 0;
    totalDownloads = 0;
}

TransfersSummaryWidget::~TransfersSummaryWidget()
{
    delete ui;
}

void TransfersSummaryWidget::paintEvent(QPaintEvent *event)
{
    updateSizes();
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);

    // 1 - draw white background
    painter.setPen(Qt::NoPen);
    painter.setBrush(brushwhitebackground);
    // limit the drawable area
    painter.setClipRect(displacement,0,displacement + width() - diaminside/2 - marginoutside,height());

    painter.drawPie(displacement + marginoutside, marginoutside, displacement + diamoutside, diamoutside, 360*4 , 360*8);
    painter.drawRect(displacement + diamoutside / 2 + marginoutside, marginoutside, displacement + this->width() - diamoutside / 2 - marginoutside, this->height() - marginoutside - 1); //-1 to fix any uneven issues
    //unlimit drawable area to allow the closing arc on the right
    painter.setClipRect(0,0,width(),height());
    painter.drawPie(displacement + this->width() - diamoutside - marginoutside, marginoutside, displacement + diamoutside, displacement + diamoutside, 360*12, 360*8);

    // 2 - draw border (except for the right side)
    // limit the drawable area
    painter.setClipRect(displacement,0,displacement + width() - diaminside/2 - marginoutside,height());
    painter.setPen(pengrey);
    painter.drawArc(displacement + marginoutside, marginoutside, displacement + diamoutside, diamoutside, 360*4 , 360*8);
    painter.drawLine(displacement + diamoutside / 2 + marginoutside, marginoutside, displacement + this->width() - diamoutside / 2 - marginoutside, marginoutside);
    painter.drawLine(displacement + diamoutside / 2 + marginoutside, this->height() - marginoutside, displacement + this->width() - diamoutside / 2 - marginoutside, this->height() - marginoutside);

    // 3 - draw speed elipses
    // limit the drawable area
    painter.setClipRect(displacement,0,displacement + width() - diaminside/2 - marginoutside,height());
    int arcx = qMin(width(), height())/28.0*34;
    int shadowwidth = qMin(width(), height())/28.0*56;
    painter.setPen(Qt::NoPen);
    painter.setBrush(brushgreyspeeds);

    painter.drawPie( displacement + arcx, margininside, displacement + diaminside, diaminside, 360*4, 360*8 );
    painter.drawRect( displacement + arcx + diaminside / 2 , margininside, displacement + shadowwidth - diaminside, diaminside);
    painter.drawPie( displacement + arcx + shadowwidth - diaminside ,margininside, displacement + diaminside, diaminside, 360*12, 360*8 );

    // drow upload arrow
    QPixmap buffer;
#if QT_VERSION >= 0x050000
    buffer.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
#endif
    buffer.load(QString::fromUtf8(":/images/upload_item_ico.png"));
    int pixmapx = arcx + minwidth / 28.0 * 8;
    int pixmapy = this->height() / 2.0 - minwidth / 28.0 * 12.0 /2.0;
    painter.drawPixmap(QRectF(pixmapx, pixmapy, minwidth / 28.0 * 12.0, minwidth / 28.0 * 12.0), buffer,  QRectF(0.0, 0.0, 12.0, 12.0));

    painter.setFont(fontUploads);
    painter.setPen(pentext);
    int texty = this->height() / 2.0 - minwidth / 28.0 * 16.0 /2.0;
    int textx = arcx + minwidth / 28.0 * (19);
    painter.drawText(QRect(textx, texty, maxWidthText , minwidth / 28.0 * 16), Qt::AlignCenter, uploadsText);


    arcx = qMin(width(), height())/28.0*94;

    painter.setPen(Qt::NoPen);
    painter.setBrush(brushgreyspeeds);
    painter.drawPie( displacement + arcx, margininside, displacement + diaminside, diaminside, 360*4, 360*8 );
    painter.drawRect( displacement + arcx + diaminside / 2 , margininside, displacement + shadowwidth - diaminside, diaminside);
    painter.drawPie( displacement + arcx + shadowwidth - diaminside ,margininside, displacement + diaminside, diaminside, 360*12, 360*8 );

    // drow download arrow
#if QT_VERSION >= 0x050000
    buffer.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
#endif
    buffer.load(QString::fromUtf8(":/images/download_item_ico.png"));
    pixmapx = arcx + minwidth / 28.0 * 8;
    pixmapy = this->height() / 2.0 - minwidth / 28.0 * 12.0 /2.0;
    painter.drawPixmap(QRectF(pixmapx, pixmapy, minwidth / 28.0 * 12.0, minwidth / 28.0 * 12.0), buffer,  QRectF(0.0, 0.0, 12.0, 12.0));

    painter.setFont(fontDownloads);
    painter.setPen(pentext);
    textx = arcx + minwidth / 28.0 * (19);
    painter.drawText(QRect(textx, texty, maxWidthText , minwidth / 28.0 * 16), Qt::AlignCenter, downloadsText);


    // 4 - draw right part of the border
    //unlimit drawable area to allow the closing arc on the right
    painter.setClipRect(0,0,width(),height());
    painter.setPen(pengrey);
    painter.drawArc(displacement + this->width() - diamoutside - marginoutside, marginoutside, displacement + diamoutside, displacement + diamoutside, 360*12, 360*8);
}

int TransfersSummaryWidget::getDisplacement() const
{
    return displacement;
}

void TransfersSummaryWidget::setDisplacement(int value)
{
    displacement = value;
    update();
}

void TransfersSummaryWidget::setPercentInnerCircle(const qreal &value)
{
    ui->bTransfersStatus->setPercentInnerCircle(value);
}

void TransfersSummaryWidget::setPercentOuterCircle(const qreal &value)
{
    ui->bTransfersStatus->setPercentOuterCircle(value);
}

// This should only be called after width() and height() are valid
void TransfersSummaryWidget::initialize()
{
    originalheight = this->height();
    originalwidth = this->width();
    minwidth = originalheight;

    calculateSpeed();

    this->shrink(true);
    updateSizes();
    update();

    QTimer::singleShot(10, this, SLOT(expand()));

}

void TransfersSummaryWidget::resizeAnimation()
{
    if (originalwidth == -1) //first time here
    {
        initialize();
    }

    if (status == Status::SHRINKING)
    {
        int prevwidth = this->width();
        qreal e = qe.elapsed();
        qreal step = pow(e/1000.0,acceleration)*speed;

        int newwidth = qMax(minwidth, qRound(originalwidth - step));
        this->setMaximumSize(newwidth, this->height());
        this->setMinimumSize(newwidth, this->height());
        if (newwidth == minwidth)
        {
            status = Status::SHRUNK;
        }
        else if (newwidth == prevwidth)
        {
            QTimer::singleShot(16, this, SLOT(resizeAnimation())); // +60 fps
        }
    }
    else if (status == Status::EXPANDING)
    {
        int prevwidth = this->width();
        qreal e = qe.elapsed();
        qreal step = pow(e/1000.0,acceleration)*speed;

        int newwidth = qMin(originalwidth, qRound(minwidth + step));
        this->setMaximumSize(newwidth, this->height());
        this->setMinimumSize(newwidth, this->height());
        if (newwidth == originalwidth)
        {
            status = Status::EXPANDED;
        }
        else if (newwidth == prevwidth)
        {
            QTimer::singleShot(16, this, SLOT(resizeAnimation())); // +60 fps
        }
    }
}

qreal TransfersSummaryWidget::getAnimationTimeMS() const
{
    return animationTimeMS;
}

void TransfersSummaryWidget::calculateSpeed()
{
    speed = (originalwidth - minwidth)/ pow(animationTimeMS/1000, acceleration);
}

void TransfersSummaryWidget::setAnimationTimeMS(const qreal &value)
{
    animationTimeMS = value;
    calculateSpeed();
}

qreal TransfersSummaryWidget::getAcceleration() const
{
    return acceleration;
}

void TransfersSummaryWidget::setAcceleration(const qreal &value)
{
    acceleration = value;
    calculateSpeed();
}

void TransfersSummaryWidget::shrink(bool noAnimate)
{
    if (noAnimate)
    {
        this->setMaximumSize(minwidth, this->height());
        this->setMinimumSize(minwidth, this->height());
        status = Status::SHRUNK;
        return;
    }
    if (status == Status::SHRUNK)
    {
        return;
    }

    if (status != Status::SHRINKING)
    {
        qe.start();
        update();

        QTimer::singleShot(1, this, SLOT(resizeAnimation()));
    }
    status = Status::SHRINKING;
}

void TransfersSummaryWidget::adjustFontSizeToText(QFont *font, int maxWidth, QString uploadText, int fontsize)
{
    int measuredWidth;
    do
    {
        font->setPixelSize(fontsize);
        QFontMetrics fm(*font);
        measuredWidth=fm.width(uploadText);
        fontsize--;
    } while (measuredWidth > maxWidth  && fontsize > 1);
}

void TransfersSummaryWidget::updateUploadsText(bool force)
{
    QString previousText = uploadsText;
    uploadsText = QString::fromUtf8("%1/%2").arg(completedUploads).arg(totalUploads);
    if (force || uploadsText.size() != previousText.size())
    {
        adjustFontSizeToText(&fontUploads, maxWidthText, uploadsText, maxFontSize);
    }
}

void TransfersSummaryWidget::updateDownloadsText(bool force)
{
    QString previousText = downloadsText;
    downloadsText = QString::fromUtf8("%1/%2").arg(completedDownloads).arg(totalDownloads);
    if (force || downloadsText.size() != previousText.size())
    {
        adjustFontSizeToText(&fontDownloads, maxWidthText, downloadsText, maxFontSize);
    }
}

void TransfersSummaryWidget::updateUploads()
{
    setPercentInnerCircle(completedUploads * 1.0 / totalUploads);
    updateUploadsText();
    update();
}

void TransfersSummaryWidget::updateDownloads()
{
    setPercentOuterCircle(completedDownloads * 1.0 / totalDownloads);
    updateDownloadsText();
    update();
}

void TransfersSummaryWidget::setTotalDownloads(int value)
{
    totalDownloads = value;
    updateDownloads();
}

void TransfersSummaryWidget::setCompletedDownloads(int value)
{
    completedDownloads = value;
    updateDownloads();
}

void TransfersSummaryWidget::setTotalUploads(int value)
{
    totalUploads = value;
    updateUploads();
}

void TransfersSummaryWidget::setCompletedUploads(int value)
{
    completedUploads = value;
    updateUploads();
}

void TransfersSummaryWidget::expand(bool noAnimate)
{
    if (noAnimate)
    {
        this->setMaximumSize(originalwidth, this->height());
        this->setMinimumSize(originalwidth, this->height());
        status = Status::EXPANDED;
        return;
    }

    if (status == Status::EXPANDED)
    {
        return;
    }

    if (status != Status::EXPANDING)
    {
        qe.start();
        update();

        QTimer::singleShot(1, this, SLOT(resizeAnimation()));
    }
    status = Status::EXPANDING;
}


void TransfersSummaryWidget::updateSizes()
{
    resizeAnimation();

    int minwidthheight = qMin(width(), height());

    if (lastwidth != this->width())
    {
        if (this->width() < minwidthheight * 2)
        {
            ui->bpause->hide();
        }
        else
        {
            ui->bpause->show();
        }
    }

    if (qMin(lastwidth, lastheigth) != minwidthheight)
    {
        wpen = qFloor(minwidthheight/28.0*1);
        spacing = minwidthheight/28.0*2;
        marginoutside = qRound(wpen/2.0);

        diamoutside = minwidthheight-marginoutside*2;
        diaminside = minwidthheight/28.0*20;

        margininside = (minwidthheight-diaminside)/2;

        residualin = 0;

        pengrey = QPen(QBrush(QColor(QString::fromUtf8("#E5E5E5"))),wpen);
        pengrey.setCapStyle(Qt::FlatCap);

        maxWidthText = minwidthheight /28.0*(56 - 18 - margininside);
        maxFontSize = minwidthheight /28.0* 12;
        updateUploadsText(true);
        updateDownloadsText(true);


        if (minwidthheight != 28 || qMin(lastwidth, lastheigth) == 28)
        {
            ui->bTransfersStatus->setMaximumHeight(minwidthheight /28.0*24);
            ui->bTransfersStatus->setMinimumHeight(minwidthheight /28.0*24);
            ui->bTransfersStatus->setMaximumWidth(minwidthheight /28.0*24);
            ui->bTransfersStatus->setMinimumWidth(minwidthheight /28.0*24);

            ui->bpause->setMaximumHeight(minwidthheight /28.0*24);
            ui->bpause->setMinimumHeight(minwidthheight /28.0*24);
            ui->bpause->setMaximumWidth(minwidthheight /28.0*24);
            ui->bpause->setMinimumWidth(minwidthheight /28.0*24);
            this->layout()->setContentsMargins(minwidthheight /28.0*2, 0, minwidthheight /28.0*2, 0);
        }
    }

    lastwidth = this->width();
    lastheigth = this->height();

}
