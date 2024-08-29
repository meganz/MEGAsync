#include "QRWidget.h"

#include <QPainter>
#include <QPainterPath>

#include "qrcodegen.h"

QRWidget::QRWidget(QWidget *parent) : QWidget(parent)
{
    text2encode = QString();
}

void QRWidget::setText2Encode(QString text)
{
    text2encode = text;
}

QRWidget::~QRWidget()
{
}

void QRWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (text2encode.isNull())
    {
        return;
    }

    QPainter painter(this);
    QColor fg("black");
    QColor bg(244, 244, 244);
    painter.setRenderHints(QPainter::Antialiasing
                           | QPainter::SmoothPixmapTransform
                           | QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fg);

    // Text data
    uint8_t qr0[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    bool ok = qrcodegen_encodeText(text2encode.toUtf8().constData(),
        tempBuffer, qr0, qrcodegen_Ecc_HIGH,
        qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
        qrcodegen_Mask_AUTO, true);

    if (!ok)
    {
        return;
    }

    const int qrLineSize = qrcodegen_getSize(qr0) > 0 ? qrcodegen_getSize(qr0) : 1;
    const double w = width();
    const double h = height();
    const double aspect = w / h;
    const double size = (aspect > 1.0) ? h : w;
    const double scale = size / (qrLineSize + 2);

    // Paint qr code without reference squares
    int wSquare = 0;
    for (int y = 0; y < qrLineSize; y++)
    {
        for (int x = 0; x < qrLineSize; x++)
        {
            bool color = qrcodegen_getModule(qr0, x, y);
            if(color)
            {
                if (wSquare &&
                        ((x > wSquare && (x < qrLineSize - wSquare || y > wSquare))
                        || (y > wSquare && y < qrLineSize - wSquare)))
                {
                    const double rx1 = (x + 1) * scale;
                    const double ry1 = (y + 1) * scale;
                    QRectF r(rx1, ry1, scale/1.5, scale/1.5);
                    painter.drawEllipse(r);
                }
            }
            else if (!wSquare)
            {
                wSquare = x; // Get width of reference squares
            }
        }
    }

    //Upper-left square
    painter.setBrush(fg);
    QPainterPath upperLeftSquare;
    upperLeftSquare.addRoundedRect(QRectF(scale, scale, scale * wSquare, scale * wSquare), 65, 65, Qt::RelativeSize);
    painter.drawPath(upperLeftSquare);

    QPainterPath upperLeftInsideSquare;
    painter.setBrush(bg);
    upperLeftInsideSquare.addRoundedRect(QRectF(scale * (wSquare - 5), scale * (wSquare - 5), scale * (wSquare - 2), scale * (wSquare - 2)), 50, 50, Qt::RelativeSize);
    painter.drawPath(upperLeftInsideSquare);

    painter.setBrush(fg);
    painter.drawEllipse(QRectF(scale * (wSquare - 4), scale * (wSquare - 4), scale * (wSquare - 4), scale * (wSquare - 4)));

    // Upper-right
    painter.setBrush(fg);
    QPainterPath upperRightSquare;
    upperRightSquare.addRoundedRect(QRectF((qrLineSize - 6) * scale, scale, scale * wSquare, scale * wSquare), 65, 65, Qt::RelativeSize);
    painter.drawPath(upperRightSquare);

    QPainterPath upperRightInsideSquare;
    painter.setBrush(bg);
    upperRightInsideSquare.addRoundedRect(QRectF((qrLineSize - 5) * scale , scale* (wSquare - 5), scale * (wSquare - 2), scale * (wSquare - 2)), 50, 50, Qt::RelativeSize);
    painter.drawPath(upperRightInsideSquare);

    painter.setBrush(fg);
    painter.drawEllipse(QRectF((qrLineSize - 4) * scale , (wSquare - 4) * scale, scale * (wSquare - 4), scale * (wSquare - 4)));

    // Bottom-left
    painter.setBrush(fg);
    QPainterPath bottomLeftSquare;
    bottomLeftSquare.addRoundedRect(QRectF(scale, (qrLineSize - 6) * scale, scale * wSquare, scale * wSquare), 65, 65, Qt::RelativeSize);
    painter.drawPath(bottomLeftSquare);

    QPainterPath bottomLeftInsideSquare;
    painter.setBrush(bg);
    bottomLeftInsideSquare.addRoundedRect(QRectF(scale * (wSquare - 5) , (qrLineSize - 5) * scale, scale * (wSquare - 2), scale * (wSquare - 2)), 50, 50, Qt::RelativeSize);
    painter.drawPath(bottomLeftInsideSquare);

    painter.setBrush(fg);
    painter.drawEllipse(QRectF(scale * (wSquare - 4), (qrLineSize - 4) * scale, scale * (wSquare - 4), scale * (wSquare - 4)));
}
