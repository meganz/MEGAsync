#ifndef ACTIVETRANSFER_H
#define ACTIVETRANSFER_H

#include <QWidget>

namespace Ui {
class ActiveTransfer;
}

class ActiveTransfer : public QWidget
{
    Q_OBJECT

public:
    explicit ActiveTransfer(QWidget *parent = 0);
    ~ActiveTransfer();

    void setFileName(QString fileName);
    void setProgress(long long completedSize, long long totalSize, bool cancellable);
    void setType(int type);
    void hideTransfer();
    bool isActive();

signals:
    void cancel(int x, int y);

private:
    Ui::ActiveTransfer *ui;

protected:
    void mouseReleaseEvent ( QMouseEvent * event );

    QString fileName;
    int type;
    bool regular;
    bool active;
};

#endif // ACTIVETRANSFER_H
