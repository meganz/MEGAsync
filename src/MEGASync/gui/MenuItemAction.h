#ifndef TRANSFERMENUITEMACTION_H
#define TRANSFERMENUITEMACTION_H

#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class MenuItemAction : public QWidgetAction
{
public:
    MenuItemAction(const QString title, const QIcon icon, QSize iconSize = QSize(24,24));
    MenuItemAction(const QString title, const QString value, const QIcon icon, QSize iconSize = QSize(24,24));
    MenuItemAction(const QString title, const QIcon icon, const QIcon hoverIcon, QSize iconSize = QSize(24,24));
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

    void setupActionWidget(QSize iconSize);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TRANSFERMENUITEMACTION_H
