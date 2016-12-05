#include "MegaSpeedGraph.h"
#include "ui_MegaSpeedGraph.h"

#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QTimer>

using namespace mega;

MegaSpeedGraph::MegaSpeedGraph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MegaSpeedGraph)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(sample()));
    megaApi = NULL;
}

void MegaSpeedGraph::init(MegaApi *megaApi, int type, int numPoints, int totalTimeMs)
{
    this->megaApi = megaApi;
    this->type = type;
    this->numPoints = numPoints;
    this->totalTimeMs = totalTimeMs;

    radius = 8;
    verticalLineColor = QColor(QString::fromUtf8("#CCCCCC"));
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        gradientColor = QColor(QString::fromUtf8("#C3E1EF"));
        graphLineColor = QColor(QString::fromUtf8("#95D0E9"));
    }
    else
    {
        gradientColor = QColor(QString::fromUtf8("#FFCCCC"));
        graphLineColor = QColor(QString::fromUtf8("#FF0000"));
    }

    clearValues();

    if (timer->isActive())
    {
        timer->start(totalTimeMs / numPoints);
    }
}

void MegaSpeedGraph::start()
{
    clearValues();
    timer->start(totalTimeMs / numPoints);
}

void MegaSpeedGraph::stop()
{
    timer->stop();
}

MegaSpeedGraph::~MegaSpeedGraph()
{
    delete ui;
}

void MegaSpeedGraph::clearValues()
{
    values.clear();
    for (int i = 0; i < numPoints; i++)
    {
        values.push_back(0);
    }
    max = 0;
    polygon.clear();
    update();
}

void MegaSpeedGraph::paintEvent(QPaintEvent *)
{
    if (!megaApi)
    {
        return;
    }

    float h = height();
    float w = width();

    // Calculate graph points if needed
    if (polygon.isEmpty())
    {
        float xStep = w / (float)(numPoints - 1);
        float x = 0;
        for (int i = 0; i < numPoints; i++)
        {
            float y = h;
            if (max)
            {
                y -= (((float)values[i] / max) * h);
            }
            polygon.push_back(QPointF(x, y));
            x += xStep;
        }

        linePath = QPainterPath();
        linePath.moveTo(polygon.first());
        for (int i = 1; i < (numPoints - 1); i++)
        {
            QPointF pt;
            QPointF pt1 = polygon.at(i);
            QPointF pt2 = polygon.at(i + 1);
            QPointF pdist = pt2 - pt1;
            float ratio = radius / sqrtf(powf(pdist.x(), 2) + pow(pdist.y(), 2));
            if (ratio > 0.5f)
            {
                ratio = 0.5f;
            }

            pt.setX((1.0f - ratio) * pt1.x() + ratio * pt2.x());
            pt.setY((1.0f - ratio) * pt1.y() + ratio * pt2.y());
            linePath.quadTo(polygon.at(i), pt);

            if (i < (numPoints - 2))
            {
                pt.setX(ratio * pt1.x() + (1.0f - ratio) * pt2.x());
                pt.setY(ratio * pt1.y() + (1.0f - ratio) * pt2.y());
                linePath.lineTo(pt);
            }
        }
        linePath.lineTo(polygon.last());
        closedLinePath = linePath;
        closedLinePath.lineTo(rect().bottomRight());
        closedLinePath.lineTo(rect().bottomLeft());
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);

    // Draw gradient
    QLinearGradient gradient(0, 0, 0, h);
    gradient.setColorAt(0, gradientColor);
    gradient.setColorAt(1, Qt::white);
    painter.setBrush(gradient);
    painter.setPen(Qt::transparent);
    painter.drawPath(closedLinePath);

    // Draw graph line
    QPen graphPen;
    graphPen.setWidth(1);
    graphPen.setColor(graphLineColor);
    painter.setPen(graphPen);
    painter.setBrush(Qt::transparent);
    painter.drawPath(linePath);

    // Draw vertical lines
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing, false);
    QPen linePen;
    linePen.setWidth(1);
    linePen.setColor(verticalLineColor);
    painter.setPen(linePen);
    float lineStep = w / 9.0;
    float lineX = 0;
    for (int i = 0; i < 10; i++)
    {
        QLineF line(QPointF(lineX, 0), QPointF(lineX, h));
        painter.drawLine(line);
        lineX += lineStep;
    }
}

void MegaSpeedGraph::sample()
{
    if (!megaApi)
    {
        return;
    }

    // Get the new value
    long long value;
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        value = megaApi->getCurrentDownloadSpeed();
    }
    else
    {
        value = megaApi->getCurrentUploadSpeed();
    }

    // Add the new value to the data vector
    values.pop_front();
    values.push_back(value);

    // Calculate max value (for autoscaling)
    max = 0;
    for (int i = 0; i< numPoints; i++)
    {
        long long val = values[i];
        if (val > max)
        {
            max = val;
        }
    }

    // Force the calculation of graph points
    polygon.clear();

    // Inform other controls about the new value
    emit newValue(value);

    // Immediate repaint
    repaint();
}
