#ifndef CIRCULARUSAGEPROGRESSBAR_H
#define CIRCULARUSAGEPROGRESSBAR_H

#include <QWidget>
#include <QPen>
#include <QIcon>

#define MAX_VALUE 100
#define MIN_VALUE 0
#define ALMOSTOVERQUOTA_VALUE 90

const char DEFAULT_FGCOLOR[] = "#5969BD";
const char DEFAULT_BKCOLOR[] = "#E5E5E5";
const char DEFAULT_OQCOLOR[] = "#F0373A";
const char DEFAULT_ALMOSTOQCOLOR[] = "#FF6F00";


class CircularUsageProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit CircularUsageProgressBar(QWidget *parent = 0);
    ~CircularUsageProgressBar();

    int getValue() const;
    void setValue(int pbValue);

    QColor getBkColor() const;
    void setBkColor(const QColor &color);

    QColor getFgColor() const;
    void setFgColor(const QColor &color);

    QColor getOqColor() const;
    void setOqColor(const QColor &pbValue);

    QColor getAlmostOqColor() const;
    void setAlmostOqColor(const QColor &value);

protected:
    void paintEvent(QPaintEvent *event);
    void drawBackgroundBar(QPainter &p, QRectF &baseRect);
    void drawArcValue(QPainter &p, const QRectF &baseRect, double arcLength);
    void drawText(QPainter &p, const QRectF &innerRect, double innerRadius, double pbValue);

    void setPenColor(QPen &pen, QColor color, bool forceRepaint = true);

    int pbValue;
    double penWidth;
    double outerRadius;

    QString textValue;

    QColor bkColor;
    QColor fgColor;
    QColor oqColor;
    QColor almostOqColor;

    QPen bkPen;
    QPen fgPen;

    QIcon mark_warning;
};

#endif // CIRCULARUSAGEPROGRESSBAR_H
