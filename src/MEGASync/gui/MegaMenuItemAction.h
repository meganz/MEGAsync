
#ifndef MEGA_MENU_ITEM_ACTION_H
#define MEGA_MENU_ITEM_ACTION_H

#include <QMenu>
#include <QWidgetAction>

class MegaMenuItem;

class MegaMenuItemAction: public QWidgetAction
{
    Q_OBJECT

public:
    explicit MegaMenuItemAction(const QString& text,
                                const QString& iconName = QString(),
                                int treeDepth = 0,
                                QObject* parent = nullptr);

    // Configuration methods
    void setIconSpacing(int spacing);
    void setTextSpacing(int spacing);
    void setBeforeIconSpacing(int spacing);
    void setItemHeight(int height);
    void setItemWidth(int width);
    void setTreeDepth(int depth);
    void setLabelText(const QString& text);
    void setActionIcon(const QString& iconName);

    // Add submenu support
    void setSubmenu(QMenu* submenu);

    QMenu* submenu() const
    {
        return m_submenu;
    }

    bool hasSubmenu() const
    {
        return m_submenu != nullptr;
    }

protected:
    QWidget* createWidget(QWidget* parent) override;
    void deleteWidget(QWidget* widget) override;

private slots:
    void onItemClicked();

private:
    QString m_text;
    QString m_iconName;
    int m_treeDepth;
    int m_iconSpacing;
    int m_textSpacing;
    int m_beforeIconSpacing;
    int m_itemHeight;
    int m_itemWidth;
    QMenu* m_submenu;
    MegaMenuItem* m_item;
};

#endif // MEGA_MENU_ITEM_ACTION_H
