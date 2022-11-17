#ifndef CUSTOMTRANSFERITEM_H
#define CUSTOMTRANSFERITEM_H

#include <QWidget>
#include <QFileInfo>
#include <QDateTime>
#include "TransferItem.h"
#include "megaapi.h"
#include "TransferRemainingTime.h"

namespace Ui {
class CustomTransferItem;
}

class CustomTransferItem : public TransferItem
{
    Q_OBJECT

public:
    explicit CustomTransferItem(QWidget *parent = 0);

    ~CustomTransferItem();

    void setFileName(QString fileName);
    void setType(int type, bool isSyncTransfer = false);
    void setTransferState(int value);
    QString getTransferName();

    bool checkIsInsideButton(QPoint pos, int button);
    void mouseHoverTransfer(bool isHover, const QPoint &pos);
    bool mouseHoverRetryingLabel(QPoint pos);

    void finishTransfer();
    void updateTransfer();
    void updateFinishedTime();
    void loadDefaultTransferIcon() {}
    void updateAnimation() {}
    bool cancelButtonClicked(QPoint/* pos*/) { return false;}
    void setStateLabel(QString labelState);

    bool eventFilter(QObject *, QEvent * ev);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

private:
    Ui::CustomTransferItem *ui;
    mega::MegaApi *megaApi;

    QString lastActionTransferIconName;
    QString lastShowInFolderIconName;

    void setActionTransferIcon(const QString &name);
    void setShowInFolderIcon(const QString &name);
    void updateFinishedIco(int transferType, bool transferErrors);

    TransferRemainingTime mTransferRemainingTime;

protected:
    bool actionButtonsEnabled;
};

#endif // CUSTOMTRANSFERITEM_H
