#ifndef PSAWIDGET_H
#define PSAWIDGET_H

#include "Utilities.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QWidget>

namespace Ui
{
class PSAwidget;
}

class PSAwidget : public QWidget
{
    Q_OBJECT

public:
    explicit PSAwidget(QWidget* parent = 0);
    ~PSAwidget();

    void setAnnounce(int id, QString title, QString desc, QString urlImage, QString textButton, QString urlClick);
    bool isPSAready();
    bool isPSAshown();
    void showPSA();
    PSA_info getPSAdata();
    void hidePSA(bool animated = false);
    void removeAnnounce();

signals:
    void PSAseen(int id);

private slots:
    void on_bMore_clicked();
    void on_bDismiss_clicked();

    void onAnimationFinished();

private:
    Ui::PSAwidget* ui;

    PSA_info info;
    bool ready;
    bool shown;

    QPropertyAnimation* minHeightAnimation;
    QPropertyAnimation* maxHeightAnimation;
    QParallelAnimationGroup* animationGroup;

    void setPSAImage(bool isValid);
};

#endif // PSAWIDGET_H
