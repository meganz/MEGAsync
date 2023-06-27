#ifndef NAMECONFLICT_H
#define NAMECONFLICT_H

#include <StalledIssueBaseDelegateWidget.h>
#include <NameConflictStalledIssue.h>
#include <StalledIssuesUtilities.h>
#include "StalledIssueActionTitle.h"

namespace Ui {
class NameConflict;
}

class NameConflictTitle : public StalledIssueActionTitle
{
    Q_OBJECT

public:
    explicit NameConflictTitle(int index, const QString &conflictedName, QWidget* parent = nullptr);
    ~NameConflictTitle() = default;

    void initTitle(const QString &conflictedName);
    int getIndex() const;

private:
    int mIndex;
};

class NameDuplicatedContainer : public QWidget
{
public:
    NameDuplicatedContainer(QWidget* parent)
        : QWidget(parent)
    {}

protected:
    void paintEvent(QPaintEvent *) override;
};

class NameConflict : public QWidget
{
    Q_OBJECT

public:
    explicit NameConflict(QWidget *parent = nullptr);
    ~NameConflict();

    void updateUi(std::shared_ptr<const NameConflictedStalledIssue> data);
    void setDisabled();

    void setDelegate(QPointer<StalledIssueBaseDelegateWidget> newDelegate);

signals:
    void refreshUi();
    void allSolved();

protected:
    virtual bool isCloud() = 0;
    virtual QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> getConflictedNames(std::shared_ptr<const NameConflictedStalledIssue> issue) = 0;
    virtual const StalledIssueDataPtr getData(std::shared_ptr<const NameConflictedStalledIssue> issue) = 0;

private slots:
    void onActionClicked(int actionId);

private:
    void removeConflictedNameWidget(QWidget *widget);

    Ui::NameConflict *ui;
    std::shared_ptr<const NameConflictedStalledIssue> mIssue;
    StalledIssuesUtilities mUtilities;
    QPointer<StalledIssueBaseDelegateWidget> mDelegate;
};

class CloudNameConflict : public NameConflict
{
    Q_OBJECT

public:
    CloudNameConflict(QWidget* parent)
        : NameConflict(parent)
    {}

protected:
    bool isCloud(){return true;}
    QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> getConflictedNames(std::shared_ptr<const NameConflictedStalledIssue> issue)
    {
        if(issue)
        {
            return issue->getNameConflictCloudData().getConflictedNames();
        }
        else
        {
            return QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>();
        }
    }
    const StalledIssueDataPtr getData(std::shared_ptr<const NameConflictedStalledIssue> issue)
    {
        if(issue)
        {
            return issue->consultCloudData();
        }
        else
        {
            return StalledIssueDataPtr();
        }
    }
};

class LocalNameConflict : public NameConflict
{
    Q_OBJECT

public:
    LocalNameConflict(QWidget* parent)
        : NameConflict(parent)
    {}

protected:
    bool isCloud(){return false;}
    QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> getConflictedNames(std::shared_ptr<const NameConflictedStalledIssue> issue)
    {
        if(issue)
        {
            return issue->getNameConflictLocalData();
        }
        else
        {
            return QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>();
        }
    }
    const StalledIssueDataPtr getData(std::shared_ptr<const NameConflictedStalledIssue> issue)
    {
        if(issue)
        {
            return issue->consultLocalData();
        }
        else
        {
            return StalledIssueDataPtr();
        }
    }
};

#endif // NAMECONFLICT_H
