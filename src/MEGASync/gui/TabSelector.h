#ifndef TABSELECTOR_H
#define TABSELECTOR_H

#include <QEvent>
#include <QPointer>
#include <QWidget>

namespace Ui
{
class TabSelector;
}

class TabSelector: public QWidget
{
    Q_OBJECT

public:
    explicit TabSelector(QWidget* parent = nullptr);
    ~TabSelector();

    Q_PROPERTY(QString title WRITE setTitle READ getTitle)
    void setTitle(const QString& title);
    QString getTitle() const;

    Q_PROPERTY(QIcon icon WRITE setIcon READ getIcon)
    void setIcon(const QIcon& icon);
    QIcon getIcon() const;

    Q_PROPERTY(QSize iconSize READ getIconSize)
    QSize getIconSize() const;

    Q_PROPERTY(bool closeButtonVisible WRITE setCloseButtonVisible READ isCloseButtonVisible)
    void setCloseButtonVisible(bool state);
    bool isCloseButtonVisible() const;

    void setCounter(int count);

    void setSelected(bool state);
    void toggleOffSiblings();

signals:
    void clicked();
    void hidden();

protected:
    bool event(QEvent* event) override;

private:
    Ui::TabSelector* ui;
    QPointer<QWidget> mTabSelectorGroupParent;
};

#endif // TABSELECTOR_H
