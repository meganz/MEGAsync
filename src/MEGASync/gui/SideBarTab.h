#ifndef SIDEBARTAB_H
#define SIDEBARTAB_H

#include <QEvent>
#include <QPointer>
#include <QWidget>

namespace Ui
{
class SideBarTab;
}

class SideBarTab: public QWidget
{
    Q_OBJECT

public:
    explicit SideBarTab(QWidget* parent = nullptr);
    ~SideBarTab();

    Q_PROPERTY(QString title WRITE setTitle READ getTitle)
    void setTitle(const QString& title);
    QString getTitle() const;

    Q_PROPERTY(QIcon icon WRITE setIcon READ getIcon)
    void setIcon(const QIcon& icon);
    QIcon getIcon() const;

    Q_PROPERTY(QSize iconSize READ getIconSize)
    QSize getIconSize() const;

    void showCloseButton();

    void setCounter(int count);

    void setSelected(bool state);
    void toggleOffSiblings();

signals:
    void clicked();
    void hidden();

protected:
    bool event(QEvent* event) override;

private:
    Ui::SideBarTab* ui;
    QPointer<QDialog> mSideBarsTopParent;
};

#endif // SIDEBARTAB_H
