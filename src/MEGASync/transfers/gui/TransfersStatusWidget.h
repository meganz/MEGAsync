#ifndef TransfersStatusWidget_H
#define TransfersStatusWidget_H

#include <QWidget>
#include <QPushButton>
#include <QPen>

namespace Ui {
class TransfersStatusWidget;
}

class TransfersStatusWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TransfersStatusWidget(QWidget *parent = 0);
    ~TransfersStatusWidget();
    void paintEvent(QPaintEvent *);

    void updateSizes();

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
    int margininside;
    int residualin; //related to the width of the pen (0 for FlatCap)
    int residualout;

    qreal percentInnerCircle;
    qreal percentOuterCircle;

    int inpoint;
    int outpoint;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    static int computePercentCircle(const qreal percentCircle, const int residual);

};

#endif // TransfersStatusWidget_H

