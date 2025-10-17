#ifndef MENUITEM_H
#define MENUITEM_H

#include "TokenizableItems/IconLabel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class MegaMenuItem: public QWidget
{
    Q_OBJECT

public:
    explicit MegaMenuItem(const QString& text,
                          const QString& iconName = QString(),
                          int treeDepth = 0,
                          bool hasSubmenu = false,
                          QWidget* parent = nullptr);

    // Spacing control methods
    void setIconSpacing(int spacing)
    {
        m_iconSpacing = spacing;
        updateLayout();
    }

    void setTextSpacing(int spacing)
    {
        m_textSpacing = spacing;
        updateLayout();
    }

    void setBeforeIconSpacing(int spacing)
    {
        m_beforeIconSpacing = spacing;
        updateLayout();
    }

    // Tree depth (affects left margin)
    void setTreeDepth(int depth);

    // Content
    void setText(const QString& text);
    void setIcon(const QString& icon);
    void setHasSubmenu(bool hasSubmenu);

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateLayout();
    void createSubmenuArrow();

private:
    QHBoxLayout* m_layout;
    IconLabel* m_iconLabel;
    QLabel* m_textLabel;
    QLabel* m_arrowLabel;

    QString m_text;
    QString m_iconName;
    int m_treeDepth;
    bool m_hasSubmenu;

    // Spacing controls
    int m_iconSpacing;
    int m_textSpacing;
    int m_beforeIconSpacing;

    // Constants
    static constexpr int DEFAULT_TREE_INDENT = 20;
    static constexpr int DEFAULT_BASE_MARGIN = 16;
    static constexpr int DEFAULT_ICON_SIZE = 16;
};

#endif // MENUITEM_H
