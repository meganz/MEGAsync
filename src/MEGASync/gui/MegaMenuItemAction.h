#ifndef MEGAMENUITEMACTION_H
#define MEGAMENUITEMACTION_H

#include <QPointer>
#include <QWidgetAction>

class MegaMenuItem;
class QMenu;

class MegaMenuItemAction: public QWidgetAction
{
    Q_OBJECT

public:
    explicit MegaMenuItemAction(const QString& text,
                                const QString& iconName = QString(),
                                int treeDepth = 0,
                                QObject* parent = nullptr);

    // Spacing control
    void setIconSpacing(int spacing);
    void setTextSpacing(int spacing);
    void setBeforeIconSpacing(int spacing);

    // Size control
    void setItemHeight(int height);
    void setItemMinimumWidth(int width);

    // Tree depth
    void setTreeDepth(int depth);

    // Content
    void setLabelText(const QString& text);
    void setActionIcon(const QString& icon);

    // Override to update submenu indicator
    void setMenu(QMenu* menu);

protected:
    QWidget* createWidget(QWidget* parent) override;
    void deleteWidget(QWidget* widget) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void paintItemBackground(MegaMenuItem* item);

private:
    QPointer<MegaMenuItem> m_item;
    QString m_text;
    QString m_iconName;
    int m_treeDepth;
    int m_iconSpacing;
    int m_textSpacing;
    int m_beforeIconSpacing;
    int m_itemHeight;
    int m_itemMinimumWidth;
    QPointer<QMenu> m_submenu;
};

#endif // MEGAMENUITEMACTION_H
