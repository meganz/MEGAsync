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
    bool getAccent() const;
    void setAccent(bool enabled);

    ~MenuItemAction();

private:
    struct Colors {static const QString Normal; static const QString Highlight; static const QString Accent;};
    bool mAccent; /* accent items will have red label text */
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
    QString getColor(); /* return color based on accent value */
};

#endif // TRANSFERMENUITEMACTION_H
