#ifndef USAGEPROGRESSBAR_H
#define USAGEPROGRESSBAR_H

#include <QWidget>

namespace Ui {
class UsageProgressBar;
}

class UsageProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit UsageProgressBar(QWidget *parent = 0);
    ~UsageProgressBar();

    void setProgress(long long valueCloud,long long valueRubbish, long long valueShares, long long valueInbox, long long totalBytes, long long totalUsed);

private:
    Ui::UsageProgressBar *ui;

protected:
    int progress;
};

#endif // USAGEPROGRESSBAR_H
