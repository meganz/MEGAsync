#ifndef RECENTFILE_H
#define RECENTFILE_H

#include <QWidget>
#include <QFileInfo>
#include <QDateTime>
#include <QMenu>
#include "TransferItem.h"

namespace Ui {
class RecentFile;
}

class RecentFile : public TransferItem
{
    Q_OBJECT

public:
    explicit RecentFile(QWidget *parent = 0);

    ~RecentFile();

    void setFileName(QString fileName);
    void setType(int type, bool isSyncTransfer = false);
    void setTransferState(int value);
    QString getTransferName();

    bool getLinkButtonClicked(QPoint pos);
    void mouseHoverTransfer(bool isHover);

    void updateTransfer() {}
    void updateFinishedTime() {}
    void loadDefaultTransferIcon() {}
    void updateAnimation() {}
    bool cancelButtonClicked(QPoint pos) { return false;}
    void setStateLabel(QString labelState) {}

    bool eventFilter(QObject *, QEvent * ev);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

signals:
    void refreshTransfer(int tag);

private:
    Ui::RecentFile *ui;

protected:
    bool getLinkButtonEnabled;

};

#endif // RECENTFILE_H
