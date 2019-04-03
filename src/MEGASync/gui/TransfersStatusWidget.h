#ifndef TransfersStatusWidget_H
#define TransfersStatusWidget_H

#include <QWidget>
#include <QPushButton>
#include <QPen>
#include <QElapsedTimer>

namespace Ui {
class TransfersStatusWidget;
}

class TransfersStatusWidget : public QWidget
{
    Q_OBJECT
public slots:
     void updateSizes();
public:
    explicit TransfersStatusWidget(QWidget *parent = 0);
    ~TransfersStatusWidget();
    void paintEvent(QPaintEvent *);

    qreal getPercentInnerCircle() const;
    void setPercentInnerCircle(const qreal &value);

    qreal getPercentOuterCircle() const;
    void setPercentOuterCircle(const qreal &value);

private:
    Ui::TransfersStatusWidget *ui;

    QPen pengrey;
    QPen penblue;
    QPen pengreen;

    int lastwidth;
    int lastheigth;

    int wpen;
    int diamoutside;
    int spacing;
    int diaminside;
    int marginoutside;
    int residualin; //related to the width of the pen (0 for FlatCap)
    int residualout;

    qreal percentInnerCircle;
    qreal percentOuterCircle;

    int inpoint;
    int outpoint;

};

#endif // TransfersStatusWidget_H

