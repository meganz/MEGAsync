#ifndef TRANSFERMENUITEMACTION_H
#define TRANSFERMENUITEMACTION_H

#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class TransferMenuItemAction : public QWidgetAction
{
public:
    TransferMenuItemAction(const QString title, const QIcon icon);
    TransferMenuItemAction(const QString title, const QIcon icon, const QIcon hoverIcon);

    ~TransferMenuItemAction();

private:
    QWidget* container;
    QIcon* icon;
    QIcon* hoverIcon;
    QPushButton* iconButton;
    QLabel* title;
    QHBoxLayout* layout;

    void setupActionWidget();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TRANSFERMENUITEMACTION_H
