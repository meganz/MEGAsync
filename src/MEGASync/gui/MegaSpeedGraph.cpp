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
    QString gradientColor;
    QString graphLineColor;
    QString verticalLineColor = QString::fromUtf8("#CCCCCC");
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        gradientColor = QString::fromUtf8("#C3E1EF");
        graphLineColor = QString::fromUtf8("#95D0E9");
    }
    else
    {
        gradientColor = QString::fromUtf8("#FFCCCC");
        graphLineColor = QString::fromUtf8("#FF0000");
    }

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
            polygon.push_back(QPoint(x, y));
            x += xStep;
        }
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing);

    // Draw gradient
    QLinearGradient gradient(0, 0, 0, h);
    gradient.setColorAt(0, gradientColor);
    gradient.setColorAt(1, Qt::white);
    polygon.push_back(rect().bottomRight());
    polygon.push_back(rect().bottomLeft());
    painter.setBrush(gradient);
    painter.setPen(Qt::transparent);
    painter.drawPolygon(polygon);
    polygon.pop_back();
    polygon.pop_back();

    // Draw graph line
    QPen graphPen;
    graphPen.setWidth(1);
    graphPen.setColor(QColor(graphLineColor));
    painter.setPen(graphPen);
    painter.setBrush(QColor(graphLineColor));
    painter.drawPolyline(polygon);

    // Draw vertical lines
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::HighQualityAntialiasing, false);
    QPen linePen;
    linePen.setWidth(1);
    linePen.setColor(QColor(verticalLineColor));
    painter.setPen(linePen);
    QBrush lineBrush;
    lineBrush.setColor(QColor(verticalLineColor));
    painter.setBrush(lineBrush);
    float lineStep = w / 9.0;
    float lineX = 0;
    for (int i = 0; i < 10; i++)
    {
        QLineF line(QPoint(lineX, 0), QPoint(lineX, h));
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
