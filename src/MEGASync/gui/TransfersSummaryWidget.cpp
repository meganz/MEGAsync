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
}

TransfersSummaryWidget::~TransfersSummaryWidget()
{
    delete ui;
}

void TransfersSummaryWidget::paintEvent(QPaintEvent *event)
{
    updateSizes();
    QPainter painter(this);


    // limit the drawable area
    painter.setClipRect(displacement,0,displacement + width() - diaminside/2 - marginoutside,height());

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    painter.setPen(pengrey);
    painter.drawArc(displacement + marginoutside, marginoutside, displacement + diamoutside, diamoutside, 360*4 , 360*8);
    painter.drawLine(displacement + diamoutside / 2 + marginoutside, marginoutside, displacement + this->width() - diamoutside / 2 - marginoutside, marginoutside);
    painter.drawLine(displacement + diamoutside / 2 + marginoutside, this->height() - marginoutside, displacement + this->width() - diamoutside / 2 - marginoutside, this->height() - marginoutside);

    // limit the drawable area
    painter.setClipRect(displacement,0,displacement + width() - diaminside - marginoutside - margininside,height());


    int arcx = qMin(width(), height())/28.0*34;
    int shadowwidth = qMin(width(), height())/28.0*56;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0,0,0,20));

    painter.drawPie( displacement + arcx, margininside, displacement + diaminside, diaminside, 360*4, 360*8 );
    painter.drawRect( displacement + arcx + diaminside / 2 , margininside, displacement + shadowwidth - diaminside, diaminside);
    painter.drawPie( displacement + arcx + shadowwidth - diaminside ,margininside, displacement + diaminside, diaminside, 360*12, 360*8 );

    arcx = qMin(width(), height())/28.0*94;

    painter.drawPie( displacement + arcx, margininside, displacement + diaminside, diaminside, 360*4, 360*8 );
    painter.drawRect( displacement + arcx + diaminside / 2 , margininside, displacement + shadowwidth - diaminside, diaminside);
    painter.drawPie( displacement + arcx + shadowwidth - diaminside ,margininside, displacement + diaminside, diaminside, 360*12, 360*8 );

    //increase drawable area to allo the closing arc on the right
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

qreal TransfersSummaryWidget::getPercentInnerCircle() const
{
    return ui->bTransfersStatus->getPercentInnerCircle();
}

void TransfersSummaryWidget::setPercentInnerCircle(const qreal &value)
{
    ui->bTransfersStatus->setPercentInnerCircle(value);
}

qreal TransfersSummaryWidget::getPercentOuterCircle() const
{
    return ui->bTransfersStatus->getPercentOuterCircle();
}

void TransfersSummaryWidget::setPercentOuterCircle(const qreal &value)
{
    ui->bTransfersStatus->setPercentOuterCircle(value);
}

void TransfersSummaryWidget::resizeAnimation()
{
    if (originalwidth == -1) //first time here
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

    if (lastwidth != this->width() || lastheigth != this->height())
    {
        int minwidthheight = qMin(this->width(), this->height());

        if (this->width() < minwidthheight * 2)
        {
            ui->bpause->hide();
        }
        else
        {
            ui->bpause->show();
        }

        wpen = qFloor(minwidthheight/28.0*2);
        diamoutside = minwidthheight-wpen;
        diaminside = minwidthheight/28.0*20;
        spacing = minwidthheight/28.0*2;
        marginoutside = wpen/2;
        margininside = (minwidthheight-diaminside)/2;

        residualin = 0;

        pengrey = QPen(QBrush(QColor(QString::fromUtf8("#F5F5F5"))),wpen); //TODO: alpha colored?
        pengrey.setCapStyle(Qt::FlatCap);

        int sizeinnterimage = minwidthheight - 4 * spacing - 4 * wpen;

        lastwidth = this->width();
        lastheigth = this->height();
    }
}
