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

    Q_PROPERTY(QString title MEMBER mTitle WRITE setTitle)
    void setTitle(const QString& title);

    Q_PROPERTY(QString iconPrefix MEMBER mIconPrefix WRITE setIconPrefix)
    void setIconPrefix(const QString& iconPrefix);

    Q_PROPERTY(int filterCriterion MEMBER mFilterCriterion READ filterCriterion WRITE setFilterCriterion)
    int filterCriterion() const;
    void setFilterCriterion(int filterCriterion);

    void setItsOn(bool itsOn);

signals:
    void tabToggled(StalledIssueFilterCriterion criterion);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;

private slots:
    void onUpdateCounter();

private:
    Q_PROPERTY(bool itsOn MEMBER mItsOn READ itsOn WRITE setItsOn)
    bool itsOn() const;

    void updateIcon();
    void toggleOffSiblings();

    Ui::StalledIssueTab *ui;
    QString mIconPrefix;
    QString mTitle;
    bool mItsOn;
    int mFilterCriterion;

    QGraphicsDropShadowEffect* mShadowTab;
};

#endif // STALLEDISSUETAB_H
