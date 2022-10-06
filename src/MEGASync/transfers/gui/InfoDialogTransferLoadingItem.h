#ifndef INFODIALOGTRANSFERLOADINGITEM_H
#define INFODIALOGTRANSFERLOADINGITEM_H

#include <QWidget>

namespace Ui {
class InfoDialogTransferLoadingItem;
}

class InfoDialogTransferLoadingItem : public QWidget
{
    Q_OBJECT

    static const QRect FullRect;

public:
    explicit InfoDialogTransferLoadingItem(QWidget *parent = nullptr);
    ~InfoDialogTransferLoadingItem();

    static QSize widgetSize();

private:
    Ui::InfoDialogTransferLoadingItem *ui;
};

#endif // INFODIALOGTRANSFERLOADINGITEM_H
