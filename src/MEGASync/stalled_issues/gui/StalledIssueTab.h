#ifndef STALLEDISSUETAB_H
#define STALLEDISSUETAB_H

#include "StalledIssue.h"

#include <QFrame>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class StalledIssueTab;
}

class StalledIssueTab : public QFrame
{
    Q_OBJECT

    static const char* HOVER_PROPERTY;

public:
    explicit StalledIssueTab(QWidget *parent = nullptr);
    ~StalledIssueTab();

    Q_PROPERTY(QString title MEMBER mTitle)
    void setTitle(const QString& title);

    Q_PROPERTY(QString iconName MEMBER mIconName WRITE setIconName)
    void setIconName(const QString& icon);

    Q_PROPERTY(int filterCriterion MEMBER mFilterCriterion READ filterCriterion WRITE setFilterCriterion)
    int filterCriterion() const;
    void setFilterCriterion(int filterCriterion);

    bool toggleTab();
    void setIsSelected(bool selected);

signals:
    void tabToggled(StalledIssueFilterCriterion criterion);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
    bool event(QEvent*) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onUpdateCounter();

private:
    Q_PROPERTY(bool isSelected MEMBER mIsSelected READ isSelected WRITE setIsSelected)
    bool isSelected() const;

    void updateIcon();
    void toggleOffSiblings();
    void createTitle();

    Ui::StalledIssueTab *ui;
    QString mIconName;
    QString mTitle;
    bool mIsSelected;
    int mFilterCriterion;

    QGraphicsDropShadowEffect* mShadowTab;
};

#endif // STALLEDISSUETAB_H
