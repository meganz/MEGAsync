#ifndef PSAWIDGET_H
#define PSAWIDGET_H

#include <QWidget>

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

private slots:
    void on_bMore_clicked();
    void on_bDismiss_clicked();

private:
    Ui::PSAwidget *ui;
    QString urlMore;
};

#endif // PSAWIDGET_H
