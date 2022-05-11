#ifndef TRANSFERMENUITEMACTION_H
#define TRANSFERMENUITEMACTION_H

#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class MenuItemAction : public QWidgetAction
{
public:
    MenuItemAction(const QString title, const QIcon icon, bool manageHoverStates = false, QSize iconSize = QSize(24,24), bool mAccent = false);
    MenuItemAction(const QString title, const QString value, const QIcon icon, bool manageHoverStates = false, QSize iconSize = QSize(24,24), bool mAccent = false);
    MenuItemAction(const QString title, const QIcon icon, const QIcon hoverIcon, bool manageHoverStates = false, QSize iconSize = QSize(24,24), bool mAccent = false);
    ~MenuItemAction();

    void setLabelText(QString title);
    void setIcon(const QIcon icon);
    void setHoverIcon(const QIcon icon);
    void setHighlight(bool mAccent);

    bool getAccent() const;
    void setAccent(bool enabled);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    struct Colors {static const QString Normal; static const QString Highlight; static const QString Accent;};
    bool mAccent; /* accent items will have red label text */
    QWidget* container;
    QIcon* icon;
    QIcon* hoverIcon;
    QPushButton* iconButton;
    QLabel* title;
    QLabel* value;
    QHBoxLayout* layout;

    void setupActionWidget(QSize iconSize, const QString& actionTitle);
    QString getColor(); /* return color based on accent value */
};

#endif // TRANSFERMENUITEMACTION_H
