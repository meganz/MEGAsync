#ifndef PSAWIDGET_H
#define PSAWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

namespace Ui {
class PSAwidget;
}

class PSAwidget : public QWidget
{
    Q_OBJECT

public:
    explicit PSAwidget(QWidget *parent = 0);
    ~PSAwidget();

    bool setAnnounce(QString title, QString desc, QString urlMore, QImage image = QImage());
    void removeAnnounce();

signals:
    void moreclicked();
    void dismissClicked();

private slots:
    void on_bMore_clicked();
    void on_bDismiss_clicked();
    void onAnimationFinished();

private:
    Ui::PSAwidget *ui;
    QString urlMore;

    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
};

#endif // PSAWIDGET_H
