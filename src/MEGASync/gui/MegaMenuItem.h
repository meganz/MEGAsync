#ifndef MENUITEM_H
#define MENUITEM_H

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOption>
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

    // State
    void setEnabled(bool enabled);

    bool isEnabled() const
    {
        return m_enabled;
    }

    void resetPressedState();

signals:
    void clicked();
    void hovered();
    void pressed();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;

private:
    void updateLayout();
    void updateStyleSheet();

private:
    void createSubmenuArrow();

private:
    QHBoxLayout* m_layout;
    QLabel* m_iconLabel;
    QLabel* m_textLabel;
    QLabel* m_arrowLabel; // Right arrow for submenus

    QString m_text;
    QString m_iconName;
    int m_treeDepth;
    bool m_hasSubmenu;

    // Spacing controls
    int m_iconSpacing; // Space between icon and text
    int m_textSpacing; // Additional text spacing
    int m_beforeIconSpacing; // Space before icon

    // State
    bool m_enabled;
    bool m_hovered;
    bool m_pressed;

    // Constants
    static constexpr int DEFAULT_TREE_INDENT = 20;
    static constexpr int DEFAULT_BASE_MARGIN = 16;
    static constexpr int DEFAULT_ICON_SIZE = 16;
};

#endif // MENUITEM_H
