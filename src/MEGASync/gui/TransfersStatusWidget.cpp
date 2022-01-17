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
        wpen = qFloor(minwidthheight/48.0*5);
        spacing = static_cast<int>(minwidthheight/48.0*2);
        marginoutside = qRound(wpen/2.0);
        diamoutside = minwidthheight-marginoutside*2;
        diaminside = static_cast<int>(diamoutside-wpen*2.0-spacing*2);
        margininside = qRound((diamoutside-diaminside)/2.0);

        residualin = 0;
        residualout = 0;

        pengrey = QPen(QBrush(QColor(QString::fromUtf8("#CCCCCC"))),wpen);
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
    percentInnerCircle = 0;
    percentOuterCircle = 0;

    updateSizes();
    installEventFilter(this);
}

TransfersStatusWidget::~TransfersStatusWidget()
{
    delete ui;
}

void TransfersStatusWidget::paintEvent(QPaintEvent*)
{
    updateSizes();
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);
    painter.setPen(pengrey);
    painter.drawArc(marginoutside+margininside, marginoutside+margininside,diaminside, diaminside, 0, 360*16);
    painter.drawArc(marginoutside, marginoutside, diamoutside, diamoutside, 0, 360*16);

    painter.setPen(penblue);
    painter.drawArc(marginoutside+margininside, marginoutside+margininside,diaminside, diaminside, 360*4, -inpoint);

    painter.setPen(pengreen);
    painter.drawArc(marginoutside, marginoutside, diamoutside, diamoutside, 360*4, -outpoint);
}


bool TransfersStatusWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        ui->bTransferManager->setIcon(QIcon(QString::fromAscii(":/images/transfer_manager.png")));
    }
    else  if (event->type() == QEvent::Leave)
    {
        ui->bTransferManager->setIcon(QIcon(QString::fromAscii(":/images/transfer_manager_greyed.png")));
    }

    return QWidget::eventFilter(obj,event);
}

qreal TransfersStatusWidget::getPercentInnerCircle() const
{
    return percentInnerCircle;
}

void TransfersStatusWidget::setPercentInnerCircle(const qreal &value)
{
    percentInnerCircle = value;
    inpoint = computePercentCircle(percentInnerCircle, residualin);
    update();
}

qreal TransfersStatusWidget::getPercentOuterCircle() const
{
    return percentOuterCircle;
}

void TransfersStatusWidget::setPercentOuterCircle(const qreal &value)
{
    percentOuterCircle = value;
    outpoint = computePercentCircle(percentOuterCircle, residualout);
    update();
}

int TransfersStatusWidget::computePercentCircle(const qreal percentCircle, const int residual)
{
    const qreal tempPoint = 360 * 16 * percentCircle;
    const qreal point = (tempPoint > residual) ? tempPoint - residual : tempPoint;
    return static_cast<int>(point);
}
