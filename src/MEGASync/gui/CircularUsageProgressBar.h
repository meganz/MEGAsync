#ifndef CIRCULARUSAGEPROGRESSBAR_H
#define CIRCULARUSAGEPROGRESSBAR_H

#include <QWidget>
#include <QPen>
#include <QIcon>

#define ALMOSTOVERQUOTA_VALUE 90

const char DEFAULT_FGCOLOR[] = "#5969BD";
const char DEFAULT_BKCOLOR[] = "#E5E5E5";
const char DEFAULT_OQCOLOR[] = "#DF4843";
const char DEFAULT_ALMOSTOQCOLOR[] = "#FF6F00";
const char DEFAULT_FGCOLOR_QUOTA[] = "#4A90E2";


class CircularUsageProgressBar : public QWidget
{
    Q_OBJECT
public:
    static const int MAXVALUE = 100;
    static const int MINVALUE = 0;


    explicit CircularUsageProgressBar(QWidget *parent = 0);
    ~CircularUsageProgressBar();

    int getValue() const;
    void setValue(int value, bool unknownTotal = false );

    QColor getBkColor() const;
    void setBkColor(const QColor &color);

    QColor getFgColor() const;
    void setFgColor(const QColor &color);

    QColor getOqColor() const;
    void setOqColor(const QColor &color);

    QColor getAlmostOqColor() const;
    void setAlmostOqColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event);
    void drawBackgroundBar(QPainter &p, QRectF &baseRect);
    void drawArcValue(QPainter &p, const QRectF &baseRect, double arcLength);
    void drawText(QPainter &p, const QRectF &innerRect, double innerRadius, double pbValue);

    void setPenColor(QPen &pen, QColor color, bool forceRepaint = true);

    int pbValue = -1;
    bool mUnknownTotal = true;
    double penWidth;
    double outerRadius;

    QString textValue;

    QColor bkColor;
    QColor fgColor;
    QColor oqColor;
    QColor almostOqColor;
    QColor currentColor;

    QPen bkPen;
    QPen fgPen;

    QIcon mark_warning;
};

#endif // CIRCULARUSAGEPROGRESSBAR_H
