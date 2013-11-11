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

    void setProgress(int value);

private:
    Ui::TransferProgressBar *ui;

protected:
    int progress;
};

#endif // TRANSFERPROGRESSBAR_H
