#include "TransfersSummaryWidget.h"
#include "ui_TransfersSummaryWidget.h"

#include <QPainter>
#include <QtMath>
#include <QDebug>
#include <QPainterPath>

TransfersSummaryWidget::TransfersSummaryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersSummaryWidget)
{
    ui->setupUi(this);
    displacement = 0;

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


void TransfersSummaryWidget::updateSizes()
{
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
