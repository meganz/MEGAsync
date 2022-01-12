#ifndef CIRCULARUSAGEPROGRESSBAR_H
#define CIRCULARUSAGEPROGRESSBAR_H

#include <QWidget>
#include <QPen>
#include <QIcon>
#include <QEvent>
#include <QPainter>

class CircularUsageProgressBar : public QWidget
{
    Q_OBJECT

public:
    static const int MAXVALUE = 100;
    static const int MINVALUE = 0;

    enum STATE
    {
        STATE_OK      = 0,
        STATE_WARNING = 1,
        STATE_OVER    = 2,
    };

    explicit CircularUsageProgressBar(QWidget* parent = 0);

    void setValue(int value);
    void setState(STATE state);
    void setTotalValueUnknown(bool isEmptyBar = true);
    void setProgressBarGradient(QColor light, QColor dark);

protected:
    void paintEvent(QPaintEvent* event);

private:
    void drawText(QPainter& p, const QRectF& innerRect, double innerRadius, double mPbValue);
    void setPenColor(QPen& pen, QColor color, bool forceRepaint = true);
    void setPenGradient(QPen& pen, QConicalGradient& gradient, bool forceRepaint = true);
    void setBarTotalValueUnkown(int value, QConicalGradient* gradient);

    int     mPbValue;
    double  mPenWidth;
    double  mOuterRadius;
    QRectF  mBaseRect;
    QString mTextValue;

    STATE mState;

    QColor mPbBgColor;

    QConicalGradient  mOkPbGradient;
    QConicalGradient  mWarnPbGradient;
    QConicalGradient  mFullPbGradient;
    QConicalGradient* mPbGradient;

    QPen mBgPen;
    QPen mFgPen;

    const QIcon mMarkWarning;
    const QIcon mMarkFull;
    const QIcon mDynTrsfOk;
    const QIcon mDynTrsfFull;
    bool  mNoTotalValue;
};

#endif // CIRCULARUSAGEPROGRESSBAR_H
