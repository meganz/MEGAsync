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
    enum {
        ALL_TRANSFERS_TAB = 0,
        DOWNLOADS_TAB   = 1,
        UPLOADS_TAB = 2,
        COMPLETED_TAB = 3
    };

    explicit ActiveTransfer(QWidget *parent = 0);
    ~ActiveTransfer();

    void setFileName(QString fileName);
    void setProgress(long long completedSize, long long totalSize, bool cancellable);
    void setType(int type);
    void hideTransfer();
    bool isActive();

signals:
    void showContextMenu(QPoint pos, bool regular);
    void openTransferManager(int tab);

private:
    Ui::ActiveTransfer *ui;

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent (QMouseEvent * event);

    QString fileName;
    int type;
    bool regular;
    bool active;
};

#endif // ACTIVETRANSFER_H
