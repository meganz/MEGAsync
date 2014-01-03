#ifndef TRANSFERPROGRESSBAR_H
#define TRANSFERPROGRESSBAR_H

#include <QWidget>

namespace Ui {
class TransferProgressBar;
}

class TransferProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit TransferProgressBar(QWidget *parent = 0);
    ~TransferProgressBar();

    void setProgress(int value, bool cancellable);

signals:
    void cancel(int x, int y);

private slots:
    void on_bCancel_clicked();

private:
    Ui::TransferProgressBar *ui;

};

#endif // TRANSFERPROGRESSBAR_H
