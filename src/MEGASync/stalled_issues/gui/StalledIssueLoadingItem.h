#ifndef STALLEDISSUELOADINGITEM_H
#define STALLEDISSUELOADINGITEM_H

#include <QWidget>

namespace Ui {
class StalledIssueLoadingItem;
}

class StalledIssueLoadingItem : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssueLoadingItem(QWidget *parent = nullptr);
    ~StalledIssueLoadingItem();

    static QSize widgetSize();

private:
    Ui::StalledIssueLoadingItem *ui;
};

#endif // STALLEDISSUELOADINGITEM_H
