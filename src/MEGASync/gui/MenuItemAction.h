#ifndef TRANSFERMENUITEMACTION_H
#define TRANSFERMENUITEMACTION_H

#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class MenuItemAction : public QWidgetAction
{
public:
    MenuItemAction(const QString& title, const QString& value,
                   const QIcon icon, const QIcon hoverIcon, bool manageHoverStates = false,
                   int treeDepth = 0, QSize iconSize = QSize(24,24));
    MenuItemAction(const QString& title, const QIcon icon, bool manageHoverStates = false,
                   int treeDepth = 0, QSize iconSize = QSize(24,24));
    MenuItemAction(const QString& title, const QString& value, const QIcon icon,
                   bool manageHoverStates = false, int treeDepth = 0,
                   QSize iconSize = QSize(24,24));
    MenuItemAction(const QString& title, const QIcon icon, const QIcon hoverIcon,
                   bool manageHoverStates = false, int treeDepth = 0,
                   QSize iconSize = QSize(24,24));
    void setLabelText(const QString&title);
    void setIcon(const QIcon icon);
    void setHoverIcon(const QIcon icon);
    void setHighlight(bool highlight);

    ~MenuItemAction();

private:
    QWidget* mContainer;
    QIcon* mIcon;
    QIcon* mHoverIcon;
    QPushButton* mIconButton;
    QLabel* mTitle;
    QLabel* mValue;
    QHBoxLayout* mLayout;
    int mTreeDepth;

    void setupActionWidget(QSize iconSize);

protected:
    bool eventFilter(QObject* obj, QEvent* event);

};

#endif // TRANSFERMENUITEMACTION_H
