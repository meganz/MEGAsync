#ifndef CIRCULARUSAGEPROGRESSBAR_H
#define CIRCULARUSAGEPROGRESSBAR_H

#include <QWidget>
#include <QPen>
#include <QIcon>
#include <QEvent>
#include <QPainter>
#include <qobjectdefs.h>

class CircularUsageProgressBar : public QWidget
{
    Q_OBJECT

public:
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

    Q_PROPERTY(QString outerCircleBackgroundColor WRITE setOuterCircleBackgroundColor NOTIFY colorChanged)
    Q_PROPERTY(QString innerCircleBackgroundColor WRITE setInnerCircleBackgroundColor NOTIFY colorChanged)
    Q_PROPERTY(QString okStateTextColor WRITE setOkStateTextColor NOTIFY colorChanged)
    Q_PROPERTY(QString lightOkProgressBarColors WRITE setLightOkProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString darkOkProgressBarColors WRITE setDarkOkProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString lightWarnProgressBarColors WRITE setLightWarnProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString darkWarnProgressBarColors WRITE setDarkWarnProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString lightFullProgressBarColors WRITE setLightFullProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString darkFullProgressBarColors WRITE setDarkFullProgressBarColor NOTIFY colorChanged)

signals:
    void colorChanged();

protected:
    void paintEvent(QPaintEvent* event);

private:
    void drawText(QPainter& p, const QRectF& innerRect, double innerRadius, double mPbValue);
    void setPenColor(QPen& pen, QColor color, bool forceRepaint = true);
    void setPenGradient(QPen& pen, QConicalGradient& gradient, bool forceRepaint = true);
    void setBarTotalValueUnkown(int value, QConicalGradient* gradient);

    void setOuterCircleBackgroundColor(const QString& color);
    void setInnerCircleBackgroundColor(const QString& color);
    void setLightOkProgressBarColor(const QString& color);
    void setDarkOkProgressBarColor(const QString& color);
    void setLightWarnProgressBarColor(const QString& color);
    void setDarkWarnProgressBarColor(const QString& color);
    void setLightFullProgressBarColor(const QString& color);
    void setDarkFullProgressBarColor(const QString& color);
    void setOkStateTextColor(const QString& color);
    void setProgressBarColors(const QString& color, STATE state, bool light);

    int     mPbValue;
    double  mPenWidth;
    double  mOuterRadius;
    QRectF  mBaseRect;
    QString mTextValue;

    STATE mState;

    QColor mPbBgColor;
    QColor mBgColor;
    QColor mOkStateTextColor;

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
