#ifndef CIRCULARUSAGEPROGRESSBAR_H
#define CIRCULARUSAGEPROGRESSBAR_H

#include <QEvent>
#include <QIcon>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QWidget>

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

    Q_PROPERTY(QString outerCircleBackgroundColor READ getOuterCircleBackgroundColor WRITE
                   setOuterCircleBackgroundColor NOTIFY colorChanged)
    Q_PROPERTY(QString innerCircleBackgroundColor READ getInnerCircleBackgroundColor WRITE
                   setInnerCircleBackgroundColor NOTIFY colorChanged)
    Q_PROPERTY(QString okStateTextColor READ getOkStateTextColor WRITE setOkStateTextColor NOTIFY
                   colorChanged)
    Q_PROPERTY(QString lightOkProgressBarColors READ getLightOkProgressBarColor WRITE
                   setLightOkProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString darkOkProgressBarColors READ getDarkOkProgressBarColor WRITE
                   setDarkOkProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString lightWarnProgressBarColors READ getLightWarnProgressBarColor WRITE
                   setLightWarnProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString darkWarnProgressBarColors READ getDarkWarnProgressBarColor WRITE
                   setDarkWarnProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString lightFullProgressBarColors READ getLightFullProgressBarColor WRITE
                   setLightFullProgressBarColor NOTIFY colorChanged)
    Q_PROPERTY(QString darkFullProgressBarColors READ getDarkFullProgressBarColor WRITE
                   setDarkFullProgressBarColor NOTIFY colorChanged)

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

    QString getOuterCircleBackgroundColor()
    {
        return {};
    }

    void setInnerCircleBackgroundColor(const QString& color);

    QString getInnerCircleBackgroundColor()
    {
        return {};
    }

    void setLightOkProgressBarColor(const QString& color);

    QString getLightOkProgressBarColor()
    {
        return {};
    }

    void setDarkOkProgressBarColor(const QString& color);

    QString getDarkOkProgressBarColor()
    {
        return {};
    }

    void setLightWarnProgressBarColor(const QString& color);

    QString getLightWarnProgressBarColor()
    {
        return {};
    }

    void setDarkWarnProgressBarColor(const QString& color);

    QString getDarkWarnProgressBarColor()
    {
        return {};
    }

    void setLightFullProgressBarColor(const QString& color);

    QString getLightFullProgressBarColor()
    {
        return {};
    }

    void setDarkFullProgressBarColor(const QString& color);

    QString getDarkFullProgressBarColor()
    {
        return {};
    }

    void setOkStateTextColor(const QString& color);

    QString getOkStateTextColor()
    {
        return {};
    }

    void setProgressBarColors(const QString& color, STATE state, bool light);

    QString getProgressBarColors()
    {
        return {};
    }

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
