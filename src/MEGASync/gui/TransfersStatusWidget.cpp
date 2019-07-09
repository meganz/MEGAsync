#include "TransfersStatusWidget.h"
#include "ui_TransfersStatusWidget.h"
#include <QPainter>
#include <QDebug>
#include <QtMath>

void TransfersStatusWidget::updateSizes()
{
    if (lastwidth != this->width() || lastheigth != this->height())
    {
        int minwidthheight = qMin(this->width(), this->height());
        wpen = qFloor(minwidthheight/48.0*4);
        diamoutside = minwidthheight-wpen;
        spacing = minwidthheight/48.0*2;
        diaminside = diamoutside-wpen*2-spacing*2;
        marginoutside = wpen/2;

        residualin = 0;
        residualout = 0;

        pengrey = QPen(QBrush(QColor(QString::fromUtf8("#DADBDB"))),wpen);
        penblue = QPen(QBrush(QColor(97,210,255)),wpen);
        pengreen = QPen(QBrush(QColor(91,217,87)),wpen);
        pengrey.setCapStyle(Qt::FlatCap);
        penblue.setCapStyle(Qt::FlatCap);
        pengreen.setCapStyle(Qt::FlatCap);

        setPercentInnerCircle(getPercentInnerCircle());
        setPercentOuterCircle(getPercentOuterCircle());

        int sizeinnterimage = minwidthheight - 4 * spacing - 4 * wpen;

        ui->bTransferManager->move(minwidthheight/2-sizeinnterimage/2,minwidthheight/2-sizeinnterimage/2);
        ui->bTransferManager->resize(sizeinnterimage, sizeinnterimage);
        ui->bTransferManager->setIconSize(QSize(sizeinnterimage, sizeinnterimage));

        lastwidth = this->width();
        lastheigth = this->height();
    }
}

TransfersStatusWidget::TransfersStatusWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersStatusWidget)
{
    ui->setupUi(this);

    lastwidth = -1;
    lastheigth = -1;

    updateSizes();
}

TransfersStatusWidget::~TransfersStatusWidget()
{
    delete ui;
}


void TransfersStatusWidget::paintEvent(QPaintEvent *event)
{
    updateSizes();
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    painter.setPen(pengrey);
    painter.drawArc(marginoutside+(diamoutside-diaminside)/2, marginoutside+(diamoutside-diaminside)/2,diaminside, diaminside, 0, 360*16);
    painter.drawArc(marginoutside, marginoutside, diamoutside, diamoutside, 0, 360*16);

    painter.setPen(penblue);
    painter.drawArc(marginoutside+(diamoutside-diaminside)/2, marginoutside+(diamoutside-diaminside)/2,diaminside, diaminside, 360*4, -inpoint);

    painter.setPen(pengreen);
    painter.drawArc(marginoutside, marginoutside, diamoutside, diamoutside, 360*4, -outpoint);
}

qreal TransfersStatusWidget::getPercentInnerCircle() const
{
    return percentInnerCircle;
}

void TransfersStatusWidget::setPercentInnerCircle(const qreal &value)
{
    percentInnerCircle = value;
    inpoint=360*16*percentInnerCircle>residualin?360*16*percentInnerCircle-residualin:360*16*percentInnerCircle;
    update();
}

qreal TransfersStatusWidget::getPercentOuterCircle() const
{
    return percentOuterCircle;
}

void TransfersStatusWidget::setPercentOuterCircle(const qreal &value)
{
    percentOuterCircle = value;
    outpoint=360*16*percentOuterCircle>residualout?360*16*percentOuterCircle-residualout:360*16*percentOuterCircle;
    update();
}
