#ifndef BALLOONTOOLTIP_H
#define BALLOONTOOLTIP_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPropertyAnimation>

namespace Ui {
class BalloonToolTip;
}

class BalloonToolTip : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(float popupOpacity READ getPopupOpacity WRITE setPopupOpacity)

public:
    explicit BalloonToolTip(QWidget *parent = nullptr);
    ~BalloonToolTip();

    void setPopupOpacity(float opacity);
    float getPopupOpacity() const;
    void attachAt(const QPoint& p);

protected:
    void paintEvent(QPaintEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

public slots:
    void setPopupText(const QString& text);
    void show();

private:
    QLabel label;
    QGridLayout layout;
    QPropertyAnimation animation;
    float popupOpacity;
};

#endif // BALLOONTOOLTIP_H
