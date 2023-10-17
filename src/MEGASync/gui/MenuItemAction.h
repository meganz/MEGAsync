#ifndef TRANSFERMENUITEMACTION_H
#define TRANSFERMENUITEMACTION_H

#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class MenuItemAction : public QWidgetAction
{
    Q_OBJECT

public:
    MenuItemAction(const QString& title, const QString& icon,
                   QObject *parent = nullptr);

    void setLabelText(const QString& title);
    void setIcon(const QIcon& icon);
    void setHighlight(bool highlight);
    bool getAccent() const;
    void setAccent(bool enabled);

    ~MenuItemAction();

    void setManagesHoverStates(bool managesHoverStates);

    void setTreeDepth(int treeDepth);

private:
    struct Colors {static const QString Normal; static const QString Highlight; static const QString Accent;};
    bool mAccent; /* accent items will have red label text */
    QWidget* mContainer;
    QLabel* mTitle;
    QLabel* mValue;
    QPushButton* mIconButton;
    QHBoxLayout* mActionLayout;
    QIcon mIcon;

    void setupActionWidget(const QSize& iconSize);

protected:
    bool eventFilter(QObject* obj, QEvent* event);
    const QString& getColor() const; /* return color based on accent value */
};

#endif // TRANSFERMENUITEMACTION_H
