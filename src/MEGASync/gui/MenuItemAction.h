#ifndef TRANSFERMENUITEMACTION_H
#define TRANSFERMENUITEMACTION_H

#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class MenuItemAction : public QWidgetAction
{
public:
    MenuItemAction(const QString title, const QIcon icon);
    MenuItemAction(const QString title, const QString value, const QIcon icon);
    MenuItemAction(const QString title, const QIcon icon, const QIcon hoverIcon);
    void setLabelText(QString title);

    ~MenuItemAction();

private:
    QWidget* container;
    QIcon* icon;
    QIcon* hoverIcon;
    QPushButton* iconButton;
    QLabel* title;
    QLabel* value;
    QHBoxLayout* layout;

    void setupActionWidget();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TRANSFERMENUITEMACTION_H
