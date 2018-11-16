#ifndef TRANSFERMANAGERITEM_H
#define TRANSFERMANAGERITEM_H

#include <QWidget>
#include <QDateTime>
#include <QMovie>
#include "TransferItem.h"

namespace Ui {
class TransferManagerItem;
}

class TransferManagerItem : public TransferItem
{
    Q_OBJECT

public:
    explicit TransferManagerItem(QWidget *parent = 0);

    void setFileName(QString fileName); 
    void setStateLabel(QString labelState);
    QString getTransferName();

    void setType(int type, bool isSyncTransfer = false);
    void setTransferState(int value);

    void finishTransfer();
    void updateTransfer();
    void updateAnimation();
    void updateFinishedTime();
    void mouseHoverTransfer(bool isHover);
    void loadDefaultTransferIcon();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    bool cancelButtonClicked(QPoint pos);
    bool mouseHoverRetryingLabel(QPoint pos);
    bool getLinkButtonClicked(QPoint pos) { return false;}

    ~TransferManagerItem();

    bool eventFilter(QObject *, QEvent * ev);

private:
    Ui::TransferManagerItem *ui;

private slots:
    void frameChanged(int);

protected:
    QMovie *animation;
    QPixmap loadIconResource;
};

#endif // TRANSFERMANAGERITEM_H
