#ifndef PSAWIDGET_H
#define PSAWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace Ui {
class PSAwidget;
}

class PSAwidget : public QWidget
{
    Q_OBJECT

public:
    explicit PSAwidget(QWidget *parent = 0);
    ~PSAwidget();

    void setAnnounce(int id, QString title, QString desc, QString urlImage, QString textButton, QString urlClick);
    int isPSAshown();

private:
    bool showPSA(QImage image = QImage());
    void removeAnnounce();

signals:
    void PSAseen(int id);
    void PSAshown();
    void PSAhidden();

private slots:
    void on_bMore_clicked();
    void on_bDismiss_clicked();
    void onAnimationFinished();

protected slots:
    void onTestTimeout();
    void onRequestImgFinished(QNetworkReply*);

private:
    Ui::PSAwidget *ui;

    int idPSA;
    QString title;
    QString desc;
    QString urlImage;
    QString textButton;
    QString urlClick;

    QNetworkAccessManager *networkAccess;
    QNetworkRequest testRequest;
    QNetworkReply *reply;
    QTimer *timer;

    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
};

#endif // PSAWIDGET_H
