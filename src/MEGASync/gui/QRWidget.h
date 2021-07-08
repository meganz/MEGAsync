#ifndef QRWIDGET_H
#define QRWIDGET_H

#include <QWidget>

class QRWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QRWidget(QWidget *parent = 0);
    void setText2Encode(QString text);

    ~QRWidget();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QString text2encode;
};

#endif // QRWIDGET_H
