#include "TransfersSummaryWidget.h"
#include "ui_TransfersSummaryWidget.h"

#include <QPainter>
#include <QtMath>
#include <QPainterPath>
#include <QTimer>
#include <QGraphicsOpacityEffect>

#include "TransferItem.h"
#include "Utilities.h"

TransfersSummaryWidget::TransfersSummaryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersSummaryWidget)
{
    ui->setupUi(this);

    status = Status::EXPANDED;
    minwidth = 28;

    animationTimeMS = 0.8*1000;
    acceleration = 0.35;
    calculateSpeed();

    fontUploads.setFamily(QString::fromUtf8("Lato"));
    fontDownloads.setFamily(QString::fromUtf8("Lato"));
    fontUploads.setBold(true);
    fontDownloads.setBold(true);

    brushspeedUp = QBrush(UPLOAD_TRANSFER_COLOR);
    brushspeedDown = QBrush(DOWNLOAD_TRANSFER_COLOR);
    brushwhitebackground = QBrush(QColor(QString::fromUtf8("#FFFFFF")));
    pentext = QPen(QColor("#FFFFFF"));

    currentDownload = 0;
    currentUpload = 0;
    totalUploads = 0;
    totalDownloads = 0;

    upArrowPixmapOrig = QIcon(QString::fromLatin1(":/images/transfer_manager/transfers_states/upload_item_ico_white.png")).pixmap(12.0, 12.0);
    dlArrowPixmapOrig = QIcon(QString::fromLatin1(":/images/transfer_manager/transfers_states/download_item_ico_white.png")).pixmap(12.0, 12.0);

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
        const char* iconFile = (paused) ? ":/images/resume.png" : ":/images/pause.png";
        const int iconSize = static_cast<int>(minwidth / 28.0 * 20);

        QIcon icon(QString::fromLatin1(iconFile));

        ui->bpause->setIcon(icon);
        ui->bpause->setIconSize(QSize(iconSize, iconSize));
    }
}

void TransfersSummaryWidget::setPauseEnabled(bool value)
{
    ui->bpause->setEnabled(value);

    if(!value)
    {
        auto effect = new QGraphicsOpacityEffect(ui->bpause);
        effect->setOpacity(0.5);
        ui->bpause->setGraphicsEffect(effect);
        ui->bpause->setAutoFillBackground(true);
    }
    else
    {
        ui->bpause->setGraphicsEffect(nullptr);
    }
}

void TransfersSummaryWidget::paintEvent(QPaintEvent*)
{
    updateSizes();
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform);

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

void TransfersSummaryWidget::setPercentUploads(long long completedBytes, long long totalBytes)
{
    if(totalBytes == 0)
    {
        return;
    }

    double percentUploads = static_cast<double>(completedBytes/* - mResetTotalUploadsBytes*/) / static_cast<double>(totalBytes);

    ui->bTransfersStatus->setPercentInnerCircle(percentUploads);
}

void TransfersSummaryWidget::setPercentDownloads(long long completedBytes, long long totalBytes)
{
    if(totalBytes == 0)
    {
        return;
    }

    double percentDownloads = static_cast<double>(completedBytes/* - mResetTotalDownloadsBytes*/)/ static_cast<double>(totalBytes);

    ui->bTransfersStatus->setPercentOuterCircle(percentDownloads);
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
    resetDownloads();
    resetUploads();

    ui->bTransfersStatus->setPercentOuterCircle(0.0);
    ui->bTransfersStatus->setPercentInnerCircle(0.0);
}

void TransfersSummaryWidget::resizeAnimation()
{
    if (originalwidth == -1) //first time here
    {
        initialize();
    }

    if (status == Status::SHRINKING)
    {
        const int previousWidth = width();
        const qreal step = computeAnimationStep();
        const int newWidth = qMax(minwidth, qRound(originalwidth - step));

        updateAnimation(previousWidth, newWidth, Status::SHRUNK);
    }
    else if (status == Status::EXPANDING)
    {
        const int previousWidth = width();
        const qreal step = computeAnimationStep();
        const int newWidth = qMin(originalwidth, qRound(minwidth + step));

        updateAnimation(previousWidth, newWidth, Status::EXPANDED);
    }
    else if (status == Status::RESIZING)
    {
        const int previousWidth = width();
        const qreal step = computeAnimationStep();
        const int newWidth = (initialwidth < goalwidth) ? qMin(goalwidth, qRound(initialwidth + step))
                                                        : qMax(goalwidth, qRound(initialwidth - step));

        updateAnimation(previousWidth, newWidth, Status::RESIZED);
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

qreal TransfersSummaryWidget::computeAnimationStep() const
{
    const qreal elapsed = static_cast<qreal>(qe.elapsed());
    return pow(elapsed /1000.0, acceleration) * speed;
}

void TransfersSummaryWidget::updateAnimation(const int previousWidth, const int newWidth,
                                             const TransfersSummaryWidget::Status newStatus)
{
    setMaximumSize(newWidth, height());
    setMinimumSize(newWidth, height());
    if (newWidth == minwidth)
    {
        status = newStatus;
    }
    else if (newWidth == previousWidth)
    {
        QTimer::singleShot(16, this, SLOT(resizeAnimation())); // +60 fps
    }
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

void TransfersSummaryWidget::mouseReleaseEvent(QMouseEvent*)
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
        measuredWidth=fm.horizontalAdvance(uploadText);
        fontsize--;
    } while (measuredWidth > maxWidth  && fontsize > 1);
}

// will return de adjusted size, override text with the final text and the position of the dots
int TransfersSummaryWidget::adjustSizeToText(QFont *font, int maxWidth, int minWidth, int margins, long long partial, long long total, int &posDotsPartial, int &posDotsTotal, QString &text, int fontsize)
{

    QString spartial = Utilities::getQuantityString(partial);
    QString stotal = Utilities::getQuantityString(total);

    text = QString::fromUtf8("%1/%2").arg(spartial).arg(stotal);

    int measuredWidth;
    font->setPixelSize(fontsize);
    QFontMetrics fm(*font);
    measuredWidth=fm.horizontalAdvance(text);
    int widthtoreturn = measuredWidth;
    int textMaxWidth = maxWidth - margins;

    if (maxWidth && measuredWidth > textMaxWidth)
    {
        posDotsPartial = spartial.size();
        posDotsTotal = stotal.size();
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
        measuredWidth=fm.horizontalAdvance(text);
    }

    return qMax(minWidth, qMin(maxWidth, widthtoreturn + margins));
}

void TransfersSummaryWidget::updateUploadsText(bool force)
{
    QString previousText = uploadsText;
    uploadsText = Utilities::getQuantityString(currentUpload) + QStringLiteral("/") + Utilities::getQuantityString(totalUploads);
    int prevUpEllipseWidth = upEllipseWidth;

    if (!currentUpload && !totalUploads)
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
            upEllipseWidth = adjustSizeToText(&fontUploads, upEllipseWidthMax, upEllipseWidthMin, fontMarginXLeft + fontMarginXRight, currentUpload, totalUploads, upPosDotsPartial, upPosDotsTotal, uploadsTextToRender, maxFontSize);
            upMaxWidthText = upEllipseWidth - (fontMarginXLeft + fontMarginXRight);
            if (prevUpEllipseWidth != upEllipseWidth)
            {
                expand();
            }
        }
    }
    else if (uploadsText != previousText)
    {
        QString spartial = Utilities::getQuantityString(currentUpload);
        QString stotal = Utilities::getQuantityString(totalUploads);
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
    downloadsText = Utilities::getQuantityString(currentDownload) + QStringLiteral("/") + Utilities::getQuantityString(totalDownloads);
    int prevDlEllipseWidth = dlEllipseWidth;

    if (!currentDownload && !totalDownloads)
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
            dlEllipseWidth = adjustSizeToText(&fontDownloads, dlEllipseWidthMax, dlEllipseWidthMin, fontMarginXLeft + fontMarginXRight, currentDownload, totalDownloads, dlPosDotsPartial, dlPosDotsTotal, downloadsTextToRender, maxFontSize);
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
        QString spartial = Utilities::getQuantityString(currentDownload);
        QString stotal = Utilities::getQuantityString(totalDownloads);
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

void TransfersSummaryWidget::setUploads(long long completed, long long total)
{
    if(total == 0)
    {
        resetUploads();
        return;
    }

    currentUpload = completed < total ? (completed + 1) : completed;
    totalUploads = total;

    updateUploads();
}

void TransfersSummaryWidget::setDownloads(long long completed, long long total)
{
    if(total == 0)
    {
        resetDownloads();
        return;
    }

    currentDownload = completed < total ? (completed + 1) : completed;
    totalDownloads = total;

    updateDownloads();
}

void TransfersSummaryWidget::resetUploads()
{
    currentUpload = 0;
    totalUploads = 0;

    updateUploads();
}

void TransfersSummaryWidget::resetDownloads()
{
    currentDownload = 0;
    totalDownloads = 0;

    updateDownloads();
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

void TransfersSummaryWidget::showEvent(QShowEvent*)
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

    if(ui->bpause->isVisible() && (currentDownload == 0 && currentUpload == 0))
    {
         ui->bpause->hide();
    }

    if (qMin(lastwidth, lastheigth) != minwidthheight)
    {
        wpen = qFloor(minwidthheight/28.0*1);
        spacing = static_cast<int>(minwidthheight/28.0*2);
        marginoutside = qRound(wpen/2.0);

        diamoutside = minwidthheight-marginoutside*2;
        diaminside = static_cast<int>(minwidthheight/28.0*20);

        firstellipseX = static_cast<int>(minwidthheight / 28.0*34);
        ellipsesMargin = static_cast<int>(minwidthheight / 28.0*4);
        afterEllipsesMargin = static_cast<int>(minwidthheight / 28.0*31);

        pixmapArrowMarginX = static_cast<int>(minwidthheight / 28.0 * 8);
        pixmapArrowY = static_cast<int>(height() / 2.0 - minwidthheight / 28.0 * 12.0 /2.0);
        pixmapWidth = static_cast<int>(minwidthheight / 28.0 * 12.0);
#if QT_VERSION >= 0x050000
        if (minwidthheight != 28.0 )
        {
            const int scaledDimension = static_cast<int>(pixmapWidth*qApp->devicePixelRatio());
            upArrowPixmap = upArrowPixmapOrig.scaled(scaledDimension, scaledDimension);
            upArrowPixmap.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
            dlArrowPixmap = dlArrowPixmapOrig.scaled(scaledDimension, scaledDimension);
            dlArrowPixmap.setDevicePixelRatio(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0);
        }
        else
#endif
        {
            upArrowPixmap = upArrowPixmapOrig;
            dlArrowPixmap = dlArrowPixmapOrig;
        }

        const int ellipseMin = static_cast<int>(minwidthheight/28.0*56);
        const int ellipseMax = static_cast<int>(minwidthheight/28.0*100);
        upEllipseWidthMin = ellipseMin;
        dlEllipseWidthMin = ellipseMin;
        upEllipseWidthMax = ellipseMax;
        dlEllipseWidthMax = ellipseMax;

        margininside = (minwidthheight-diaminside)/2;

        residualin = 0;

        pengrey = QPen(QBrush(QColor(QString::fromUtf8("#E5E5E5"))),wpen);
        pengrey.setCapStyle(Qt::FlatCap);

        maxFontSize = static_cast<int>(minwidthheight /28.0* 12);
        fontMarginXLeft = static_cast<int>(minwidthheight / 28.0 * 22);
        fontMarginXRight = static_cast<int>(minwidthheight / 28.0 * 8);
        fontHeight = static_cast<int>(maxFontSize * 4.0 / 3.0);
        fontY = static_cast<int>(height() / 2.0 - fontHeight /2.0);


        updateUploadsText(true);
        updateDownloadsText(true);

        if (minwidthheight != 28 || qMin(lastwidth, lastheigth) == 28)
        {
            const int minimumDimension = static_cast<int>(minwidthheight/28.0 * 24);
            ui->bTransfersStatus->setMaximumHeight(minimumDimension);
            ui->bTransfersStatus->setMinimumHeight(minimumDimension);
            ui->bTransfersStatus->setMaximumWidth(minimumDimension);
            ui->bTransfersStatus->setMinimumWidth(minimumDimension);

            ui->bpause->setMaximumHeight(minimumDimension);
            ui->bpause->setMinimumHeight(minimumDimension);
            ui->bpause->setMaximumWidth(minimumDimension);
            ui->bpause->setMinimumWidth(minimumDimension);

            const int minimumLayoutDimension = static_cast<int>(minwidthheight/28.0 * 2);
            layout()->setContentsMargins(minimumLayoutDimension, 0, minimumLayoutDimension, 0);
        }
        const int iconSize = static_cast<int>(minwidth / 28.0 * 20);
        ui->bpause->setIconSize(QSize(iconSize, iconSize));

    }

    lastwidth = this->width();
    lastheigth = this->height();
}
