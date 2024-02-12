#ifndef NAMECONFLICT_H
#define NAMECONFLICT_H

#include <StalledIssueBaseDelegateWidget.h>
#include <NameConflictStalledIssue.h>
#include <StalledIssuesUtilities.h>
#include "StalledIssueActionTitle.h"

namespace Ui {
class NameConflict;
}

class NameDuplicatedContainer : public QWidget
{
public:
    NameDuplicatedContainer(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName(QLatin1String("DuplicatedContainer"));
    }

protected:
    void paintEvent(QPaintEvent *) override;
};

class NameConflict : public QWidget
{
    Q_OBJECT

public:
    explicit NameConflict(QWidget *parent = nullptr);
    virtual ~NameConflict();

    void updateUi(std::shared_ptr<const NameConflictedStalledIssue> data);
    void setDisabled();

    void setDelegate(QPointer<StalledIssueBaseDelegateWidget> newDelegate);

    virtual QString getConflictedName(std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info) const;

signals:
    void refreshUi();
    void allSolved();

protected:
    std::shared_ptr<const NameConflictedStalledIssue> mIssue;

    virtual bool isCloud() = 0;
    virtual QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> getConflictedNamesInfo() = 0;
    virtual const StalledIssueDataPtr getData() = 0;

private slots:
    void onActionClicked(int actionId);
    void onRawInfoChecked();

private:
    void initTitle(StalledIssueActionTitle* title, int index, const QString& conflictedName);
    void initActionButtons(StalledIssueActionTitle* title);
    void updateTitleExtraInfo(StalledIssueActionTitle* title, std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info);

    Ui::NameConflict *ui;
    StalledIssuesUtilities mUtilities;
    QPointer<StalledIssueBaseDelegateWidget> mDelegateWidget;
    QMap<int, QPointer<StalledIssueActionTitle>> mTitlesByIndex;
    QMap<int, QPointer<QWidget>> mContainerByDuplicateByGroupId;
};

class CloudNameConflict : public NameConflict
{
    Q_OBJECT

public:
    CloudNameConflict(QWidget* parent)
        : NameConflict(parent)
    {}

    ~CloudNameConflict() override {}

protected:
    bool isCloud(){return true;}
    QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> getConflictedNamesInfo()
    {
        if(mIssue)
        {
            return mIssue->getNameConflictCloudData().getConflictedNames();
        }
        else
        {
            return QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>();
        }
    }

    QString getConflictedName(std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo> info) const override
    {
        return mIssue->getNameConflictCloudData().getConflictedName(info);
    }

    const StalledIssueDataPtr getData()
    {
        if(mIssue)
        {
            return mIssue->consultCloudData();
        }
        else
        {
            return StalledIssueDataPtr();
        }
    }


private:
    QMap<QString, std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> mIssuesByFingerprint;
};

class LocalNameConflict : public NameConflict
{
    Q_OBJECT

public:
    LocalNameConflict(QWidget* parent)
        : NameConflict(parent)
    {}

    ~LocalNameConflict() override {}

protected:
    bool isCloud(){return false;}
    QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>> getConflictedNamesInfo()
    {
        if(mIssue)
        {
            return mIssue->getNameConflictLocalData();
        }
        else
        {
            return QList<std::shared_ptr<NameConflictedStalledIssue::ConflictedNameInfo>>();
        }
    }
    const StalledIssueDataPtr getData()
    {
        if(mIssue)
        {
            return mIssue->consultLocalData();
        }
        else
        {
            return StalledIssueDataPtr();
        }
    }
};

#endif // NAMECONFLICT_H
