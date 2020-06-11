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
//    ui->bTransfersStatus->setAttribute(Qt::WA_TransparentForMouseEvents);

    status = Status::EXPANDED;
    minwidth = 28;

    animationTimeMS = 0.8*1000;
    acceleration = 0.35;
    calculateSpeed();

    fontUploads.setFamily(QString::fromUtf8("Lato"));
    fontDownloads.setFamily(QString::fromUtf8("Lato"));
    fontUploads.setBold(true);
    fontDownloads.setBold(true);

    brushspeedUp = QBrush(QColor(QString::fromUtf8("#2BA6DE")));
    brushspeedDown = QBrush(QColor(QString::fromUtf8("#31B500")));
    brushwhitebackground = QBrush(QColor(QString::fromUtf8("#FFFFFF")));
    pentext = QPen(QColor("#FFFFFF"));

    completedDownloads = 0;
    completedUploads = 0;
    totalUploads = 0;
    totalDownloads = 0;


    upArrowPixmapOrig = QIcon(QString::fromUtf8(":/images/upload_item_ico_white.png")).pixmap(12.0, 12.0);
    dlArrowPixmapOrig = QIcon(QString::fromUtf8(":/images/download_item_ico_white.png")).pixmap(12.0, 12.0);

    connect(ui->bpause, SIGNAL(clicked()), this, SIGNAL(pauseResumeClicked()));
}

TransfersSummaryWidget::~TransfersSummaryWidget()
{
    delete ui;
}

void TransfersSummaryWidget::drawEllipse(int x, int y,  int diam, int width, QPainter *painter)
{
    painter->drawPie(  x, y,  diam, diam, 360*4, 360*8 );
    painter->drawRect(  x + diam / 2 - 1 , y,  width - diam + 2, diam); //Notice we add one pixel per side to ensure no gaps are left
    painter->drawPie(  x + width - diam ,y,  diam, diam, 360*12, 360*8 );
}

void TransfersSummaryWidget::setPaused(bool value)
{
    if (value != paused)
    {
        paused = value;
        if (paused)
        {
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/resume.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bpause->setIcon(icon);
            ui->bpause->setIconSize(QSize(minwidth / 28.0 * 20, minwidth / 28.0 * 20));
        }
        else
        {
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/pause.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->bpause->setIcon(icon);
            ui->bpause->setIconSize(QSize(minwidth / 28.0 * 20, minwidth / 28.0 * 20));

        }
    }
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
    painter.setClipRect(0,0, width() - diaminside/2 - marginoutside,height());

    painter.drawPie( marginoutside, marginoutside,  diamoutside, diamoutside, 360*4 , 360*8);
    painter.drawRect( diamoutside / 2 + marginoutside, marginoutside,  this->width() - diamoutside / 2 - marginoutside, this->height() - marginoutside - 1); //-1 to fix any uneven issues
    //unlimit drawable area to allow the closing arc on the right
    painter.setClipRect(0,0,width(),height());
    painter.drawPie( this->width() - diamoutside - marginoutside, marginoutside,  diamoutside,  diamoutside, 360*12, 360*8);

    // 2 - draw border (except for the right side)
    // limit the drawable area
    painter.setClipRect(0,0, width() - diaminside/2 - marginoutside,height());
    painter.setPen(pengrey);
    painter.drawArc( marginoutside, marginoutside,  diamoutside, diamoutside, 360*4 , 360*8);
    painter.drawLine( diamoutside / 2 + marginoutside, marginoutside,  this->width() - diamoutside / 2 - marginoutside, marginoutside);
    painter.drawLine( diamoutside / 2 + marginoutside, this->height() - marginoutside,  this->width() - diamoutside / 2 - marginoutside, this->height() - marginoutside);

    // 3 - draw speed elipses
    // limit the drawable area
    painter.setClipRect(0,0, width() - diaminside/2 - marginoutside,height());

    int arcx = firstellipseX;
    if (upEllipseWidth)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(brushspeedUp);
        drawEllipse(arcx, margininside, diaminside, upEllipseWidth, &painter);

        // drow upload arrow
        int pixmapx = arcx + pixmapArrowMarginX;
        int pixmapy = pixmapArrowY;
        painter.drawPixmap(QPoint(pixmapx, pixmapy), upArrowPixmap);

        painter.setFont(fontUploads);
        painter.setPen(pentext);
        int texty = fontY;
        int textx = arcx + fontMarginXLeft;
        painter.drawText(QRect(textx, texty, upMaxWidthText , fontHeight), Qt::AlignCenter, uploadsTextToRender);
    }

    if (dlEllipseWidth)
    {
        arcx = arcx + upEllipseWidth + (upEllipseWidth?ellipsesMargin:0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(brushspeedDown);
        drawEllipse(arcx, margininside, diaminside, dlEllipseWidth, &painter);

        // drow download arrow
        int pixmapx = arcx + pixmapArrowMarginX;
        int pixmapy = pixmapArrowY;
        painter.drawPixmap(QPoint(pixmapx, pixmapy), dlArrowPixmap);

        painter.setFont(fontDownloads);
        painter.setPen(pentext);
        int texty = fontY;
        int textx = arcx + fontMarginXLeft;
        painter.drawText(QRect(textx, texty, dlMaxWidthText , fontHeight), Qt::AlignCenter, downloadsTextToRender);

    }

    // 4 - draw right part of the border
    //unlimit drawable area to allow the closing arc on the right
    painter.setClipRect(0,0,width(),height());
    painter.setPen(pengrey);
    painter.drawArc( this->width() - diamoutside - marginoutside, marginoutside,  diamoutside,  diamoutside, 360*12, 360*8);

    neverPainted = false;
}

void TransfersSummaryWidget::setPercentUploads(const qreal &value)
{
    ui->bTransfersStatus->setPercentInnerCircle(value);
}

void TransfersSummaryWidget::setPercentDownloads(const qreal &value)
{
    ui->bTransfersStatus->setPercentOuterCircle(value);
}

// This should only be called after width() and height() are valid
void TransfersSummaryWidget::showAnimated()
{
    this->shrink(true);
    QTimer::singleShot(10, this, SLOT(expand()));
}

void TransfersSummaryWidget::initialize()
{
    originalheight = this->height();
    originalwidth = this->width();
    minwidth = originalheight;
    neverPainted = true;

    calculateSpeed();

    showAnimated();

    updateSizes();
    update();


}

void TransfersSummaryWidget::reset()
{
    setCompletedDownloads(0);
    setCompletedUploads(0);
    setTotalDownloads(0);
    setTotalUploads(0);
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
    else if (status == Status::RESIZING)
    {
        int prevwidth = this->width();
        qreal e = qe.elapsed();

        qreal step = pow(e/1000.0,acceleration)*speed;

        int newwidth = (initialwidth < goalwidth) ? qMin(goalwidth, qRound(initialwidth + step)) : qMax(goalwidth, qRound(initialwidth - step));
        this->setMaximumSize(newwidth, this->height());
        this->setMinimumSize(newwidth, this->height());
        if (newwidth == goalwidth)
        {
            status = Status::RESIZED;
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

void TransfersSummaryWidget::calculateSpeed(int initWidth, int endWidth)
{
    if (initWidth == -1)
    {
        initWidth = minwidth;
    }
    if (endWidth == -1)
    {
        endWidth = originalwidth;
    }
    speed = abs(initWidth - endWidth)/ pow(animationTimeMS/1000, acceleration);
}


bool TransfersSummaryWidget::isWithinPseudoEllipse(QPoint pos, int x, int y, int w, int diam)
{
    if (QRect(  x + diam / 2 , y,  w - diam , diam).contains(pos))
    {
        return true;
    }

    if ( sqrt(pow( pos.x() - ( x + diam/2) , 2)+pow( pos.y() - (y + diam/2.0) , 2)) < (diam/2.0))
    {
        return true;
    }

    if ( sqrt(pow( pos.x() - ( x + w - diam/2) , 2)+pow( pos.y() - (y + diam/2.0) , 2)) < (diam/2.0))
    {
        return true;
    }

    return false;
}

void TransfersSummaryWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = this->mapFromGlobal(QCursor::pos());

#ifndef Q_OS_MACX
    if (isWithinPseudoEllipse(pos, marginoutside, marginoutside,  this->width() - 2 * marginoutside, diamoutside))
    {
        this->setCursor(Qt::PointingHandCursor);
    }
    else
    {
        this->setCursor(Qt::ArrowCursor);
    }
#endif
    int arcx = firstellipseX;

    if (upEllipseWidth && isWithinPseudoEllipse(pos, arcx, margininside,  upEllipseWidth, diaminside))
    {
        emit upAreaHovered(event);
        return;
    }

    arcx = firstellipseX + upEllipseWidth + (upEllipseWidth?ellipsesMargin:0);
    if (dlEllipseWidth && isWithinPseudoEllipse(pos, arcx, margininside,  dlEllipseWidth, diaminside))
    {
        emit dlAreaHovered(event);
        return;
    }

    if (isWithinPseudoEllipse(pos, marginoutside, marginoutside,  this->width() - 2 * marginoutside, diamoutside))
    {
        if ((!upEllipseWidth && !dlEllipseWidth) || sqrt(pow( pos.x() - (ui->bpause->x() + ui->bpause->size().width() / 2.0),2.0)
                          + pow( pos.y() - (ui->bpause->y() + ui->bpause->size().height() / 2.0), 2.0))
                          > (ui->bpause->iconSize().width()/2.0) )

        {
            emit generalAreaHovered(event);
        }
        else
        {
            emit pauseResumeHovered(event);
        }
    }
}

void TransfersSummaryWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint pos = this->mapFromGlobal(QCursor::pos());

    int arcx = firstellipseX;

    if (upEllipseWidth && isWithinPseudoEllipse(pos, arcx, margininside,  upEllipseWidth, diaminside))
    {
        emit upAreaClicked();
        return;
    }

    arcx = firstellipseX + upEllipseWidth + (upEllipseWidth?ellipsesMargin:0);
    if (dlEllipseWidth && isWithinPseudoEllipse(pos, arcx, margininside,  dlEllipseWidth, diaminside))
    {
        emit dlAreaClicked();
        return;
    }

    if (isWithinPseudoEllipse(pos, marginoutside, marginoutside,  this->width() - 2 * marginoutside, diamoutside))
    {
        //This is not necessary: bpause captures the click otherwise
//        if ((!upEllipseWidth && !dlEllipseWidth) || sqrt(pow( pos.x() - (ui->bpause->x() + ui->bpause->size().width() / 2.0),2.0)
//                          + pow( pos.y() - (ui->bpause->y() + ui->bpause->size().height() / 2.0), 2.0))
//                          > (ui->bpause->iconSize().width()/2.0) )

        {
            emit generalAreaClicked();
        }
    }
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
    int goalwidth = minwidth;
    return doResize(goalwidth, noAnimate);
}


void TransfersSummaryWidget::doResize(int futureWidth, bool noAnimate)
{
    if (noAnimate)
    {
        this->setMaximumSize(futureWidth, this->height());
        this->setMinimumSize(futureWidth, this->height());
        status = Status::RESIZED;
        return;
    }
    if (futureWidth == width())
    {
        status = Status::RESIZED;
        return;
    }

    if (status != Status::RESIZING)
    {
        qe.start();
        update();

        QTimer::singleShot(1, this, SLOT(resizeAnimation()));
    }
    goalwidth = futureWidth;
    initialwidth = width();
    calculateSpeed(goalwidth, initialwidth);
    status = Status::RESIZING;
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

// will return de adjusted size, override text with the final text and the position of the dots
int TransfersSummaryWidget::adjustSizeToText(QFont *font, int maxWidth, int minWidth, int margins, long long partial, long long total, int &posDotsPartial, int &posDotsTotal, QString &text, int fontsize)
{

    QString spartial = QString::fromUtf8("%1").arg(partial);
    QString stotal = QString::fromUtf8("%1").arg(total);

    text = QString::fromUtf8("%1/%2").arg(spartial).arg(stotal);

    QString::fromUtf8("%1/%2").arg(spartial).arg(stotal);

    int measuredWidth;
    font->setPixelSize(fontsize);
    QFontMetrics fm(*font);
    measuredWidth=fm.width(text);
    int widthtoreturn = measuredWidth;
    int textMaxWidth = maxWidth - margins;

    if (maxWidth && measuredWidth > textMaxWidth)
    {
        posDotsPartial = QString::fromUtf8("%1").arg(partial).size();
        posDotsTotal = QString::fromUtf8("%1").arg(total).size();
    }
    else
    {
        posDotsPartial = 0;
        posDotsTotal = 0;
    }

    while (maxWidth && measuredWidth > textMaxWidth && posDotsPartial > 0 && posDotsTotal > 0 )
    {
        if (spartial.size() > stotal.size())
        {
            posDotsPartial--;
            spartial = spartial.mid(0, posDotsPartial) + QString::fromUtf8("...") + spartial.mid(spartial.size() - trailingChars);
        }
        else
        {
            posDotsTotal--;
            stotal = stotal.mid(0, posDotsTotal) + QString::fromUtf8("...") + stotal.mid(stotal.size() - trailingChars);
        }

        text = QString::fromUtf8("%1/%2").arg(spartial).arg(stotal);
        measuredWidth=fm.width(text);
    }

    return qMax(minWidth, qMin(maxWidth, widthtoreturn + margins));
}

void TransfersSummaryWidget::updateUploadsText(bool force)
{
    QString previousText = uploadsText;
    uploadsText = QString::fromUtf8("%1/%2").arg(completedUploads).arg(totalUploads);
    int prevUpEllipseWidth = upEllipseWidth;

    if (!completedUploads && !totalUploads)
    {
        upEllipseWidth = 0;
        if (prevUpEllipseWidth != upEllipseWidth)
        {
            expand();
        }
        return;
    }

    if (force || uploadsText.size() != previousText.size() || !upEllipseWidth)
    {
        uploadsTextToRender = uploadsText;

        if (upEllipseWidthMax)
        {
            upEllipseWidth = adjustSizeToText(&fontUploads, upEllipseWidthMax, upEllipseWidthMin, fontMarginXLeft + fontMarginXRight, completedUploads, totalUploads, upPosDotsPartial, upPosDotsTotal, uploadsTextToRender, maxFontSize);
            upMaxWidthText = upEllipseWidth - (fontMarginXLeft + fontMarginXRight);
            if (prevUpEllipseWidth != upEllipseWidth)
            {
                expand();
            }
        }
    }
    else if (uploadsText != previousText)
    {
        QString spartial = QString::fromUtf8("%1").arg(completedUploads);
        QString stotal = QString::fromUtf8("%1").arg(totalUploads);
        if (upPosDotsPartial && upPosDotsPartial < spartial.size())
        {
            spartial = spartial.mid(0, upPosDotsPartial) + QString::fromUtf8("...") + spartial.mid(spartial.size() - trailingChars);
        }
        if (upPosDotsTotal && upPosDotsTotal < stotal.size())
        {
            stotal = stotal.mid(0, upPosDotsTotal) + QString::fromUtf8("...") + stotal.mid(stotal.size() - trailingChars);
        }
        uploadsTextToRender = QString::fromUtf8("%1/%2").arg(spartial).arg(stotal);
    }
}

void TransfersSummaryWidget::updateDownloadsText(bool force)
{
    QString previousText = downloadsText;
    downloadsText = QString::fromUtf8("%1/%2").arg(completedDownloads).arg(totalDownloads);
    int prevDlEllipseWidth = dlEllipseWidth;

    if (!completedDownloads && !totalDownloads)
    {
        dlEllipseWidth = 0;
        if (prevDlEllipseWidth != dlEllipseWidth)
        {
            if (isVisible())
            {
                expand();
            }
        }
        return;
    }

    if (force || downloadsText.size() != previousText.size() || !dlEllipseWidth)
    {
        downloadsTextToRender = downloadsText;

        if (dlEllipseWidthMax)
        {
            dlEllipseWidth = adjustSizeToText(&fontDownloads, dlEllipseWidthMax, dlEllipseWidthMin, fontMarginXLeft + fontMarginXRight, completedDownloads, totalDownloads, dlPosDotsPartial, dlPosDotsTotal, downloadsTextToRender, maxFontSize);
            dlMaxWidthText = dlEllipseWidth - (fontMarginXLeft + fontMarginXRight);
            if (prevDlEllipseWidth != dlEllipseWidth)
            {
                if (isVisible())
                {
                    expand();
                }
            }
        }
    }
    else if (downloadsText != previousText)
    {
        QString spartial = QString::fromUtf8("%1").arg(completedDownloads);
        QString stotal = QString::fromUtf8("%1").arg(totalDownloads);
        if (dlPosDotsPartial && dlPosDotsPartial < spartial.size())
        {
            spartial = spartial.mid(0, dlPosDotsPartial) + QString::fromUtf8("...") + spartial.mid(spartial.size() - trailingChars);
        }
        if (dlPosDotsTotal && dlPosDotsTotal < stotal.size())
        {
            stotal = stotal.mid(0, dlPosDotsTotal) + QString::fromUtf8("...") + stotal.mid(stotal.size() - trailingChars);
        }
        downloadsTextToRender = QString::fromUtf8("%1/%2").arg(spartial).arg(stotal);
    }
}

void TransfersSummaryWidget::updateUploads()
{
    updateUploadsText();
    update();
}

void TransfersSummaryWidget::updateDownloads()
{
    updateDownloadsText();
    update();
}

void TransfersSummaryWidget::setTotalDownloads(long long  value)
{
    if (totalDownloads != value)
    {
        totalDownloads = value;
        updateDownloads();
    }
}

void TransfersSummaryWidget::setCompletedDownloads(long long  value)
{
    if (completedDownloads != value)
    {
        completedDownloads = value;
        updateDownloads();
    }
}

void TransfersSummaryWidget::setTotalUploads(long long  value)
{
    if (totalUploads != value)
    {
        totalUploads = value;
        updateUploads();
    }
}

void TransfersSummaryWidget::setCompletedUploads(long long  value)
{
    if (completedUploads != value)
    {
        completedUploads = value;
        updateUploads();
    }
}

void TransfersSummaryWidget::expand(bool noAnimate)
{
    int goalwidth = firstellipseX + upEllipseWidth + ((upEllipseWidth && dlEllipseWidth)?ellipsesMargin:0) + dlEllipseWidth + afterEllipsesMargin;

    if (!upEllipseWidth && !dlEllipseWidth)
    {
        goalwidth = minwidth;
    }

    return doResize(goalwidth, noAnimate);
}

void TransfersSummaryWidget::showEvent(QShowEvent *event)
{
    expand(); //This will trigger an animation if size has changed
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

        firstellipseX = minwidthheight / 28.0*34;
        ellipsesMargin = minwidthheight / 28.0*4;
        afterEllipsesMargin = minwidthheight / 28.0*31;

        pixmapArrowMarginX = minwidthheight / 28.0 * 8;
        pixmapArrowY = this->height() / 2.0 - minwidthheight / 28.0 * 12.0 /2.0;
        pixmapWidth = minwidthheight / 28.0 * 12.0;
#if QT_VERSION >= 0x050000
        if (minwidthheight != 28.0 )
        {
            upArrowPixmap = upArrowPixmapOrig.scaled(pixmapWidth*qApp->devicePixelRatio(), pixmapWidth*qApp->devicePixelRatio());
            upArrowPixmap.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
            dlArrowPixmap = dlArrowPixmapOrig.scaled(pixmapWidth*qApp->devicePixelRatio(), pixmapWidth*qApp->devicePixelRatio());
            dlArrowPixmap.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
        }
        else
#endif
        {
            upArrowPixmap = upArrowPixmapOrig;
            dlArrowPixmap = dlArrowPixmapOrig;
        }

        upEllipseWidthMin = minwidthheight/28.0*56;
        dlEllipseWidthMin = minwidthheight/28.0*56;
        upEllipseWidthMax = minwidthheight/28.0*100;
        dlEllipseWidthMax = minwidthheight/28.0*100;

        margininside = (minwidthheight-diaminside)/2;

        residualin = 0;

        pengrey = QPen(QBrush(QColor(QString::fromUtf8("#E5E5E5"))),wpen);
        pengrey.setCapStyle(Qt::FlatCap);

        maxFontSize = minwidthheight /28.0* 12;
        fontMarginXLeft = minwidthheight / 28.0 * 22;
        fontMarginXRight = minwidthheight / 28.0 * 8;
        fontHeight = maxFontSize * 4.0 / 3.0;
        fontY = this->height() / 2.0 - fontHeight /2.0;


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
        ui->bpause->setIconSize(QSize(minwidth / 28.0 * 20, minwidth / 28.0 * 20));

    }

    lastwidth = this->width();
    lastheigth = this->height();

}
