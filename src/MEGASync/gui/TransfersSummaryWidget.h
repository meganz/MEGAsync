#ifndef TRANSFERSSUMMARYWIDGET_H
#define TRANSFERSSUMMARYWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPen>


namespace Ui {
class TransfersSummaryWidget;
}

class TransfersSummaryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransfersSummaryWidget(QWidget *parent = 0);
    ~TransfersSummaryWidget();

    void updateSizes();
    void paintEvent(QPaintEvent *event);
    int getDisplacement() const;
    void setDisplacement(int value);

private:
    Ui::TransfersSummaryWidget *ui;

    QPen pengrey;

    int lastwidth;
    int lastheigth;

    int wpen;
    int diamoutside;
    int diaminside;
    int spacing;
    int marginoutside;
    int margininside;
    int residualin; //related to the width of the pen (0 for FlatCap)
    int displacement;

};


#endif // TRANSFERSSUMMARYWIDGET_H
