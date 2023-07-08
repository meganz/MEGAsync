#ifndef STALLEDISSUECHOOSEWIDGET_H
#define STALLEDISSUECHOOSEWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"
#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPointer>

namespace Ui {
class StalledIssueChooseWidget;
}

class StalledIssueChooseWidget : public QFrame
{
    Q_OBJECT

public:
    explicit StalledIssueChooseWidget(QWidget *parent = nullptr);
    virtual ~StalledIssueChooseWidget();

    void updateUi(StalledIssueDataPtr data, LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side);
    const StalledIssueDataPtr& data();

signals:
    void chooseButtonClicked(int id);

protected:
    virtual QString movedToBinText() const = 0;
    bool eventFilter(QObject *watched, QEvent *event) override;
    Ui::StalledIssueChooseWidget *ui;

private slots:
    void onActionClicked(int button_id);

private:
    void setSolved();

    StalledIssueDataPtr mData;
    QPointer<QGraphicsOpacityEffect> mDisableEffect;
};

class LocalStalledIssueChooseWidget : public StalledIssueChooseWidget
{
public:
    explicit LocalStalledIssueChooseWidget(QWidget *parent = nullptr)
        : StalledIssueChooseWidget(parent)
    {}

    ~LocalStalledIssueChooseWidget() = default;

    QString movedToBinText() const override;
    void updateUi(LocalStalledIssueDataPtr localData, LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side);
};

class CloudStalledIssueChooseWidget : public StalledIssueChooseWidget
{
public:
    explicit CloudStalledIssueChooseWidget(QWidget *parent = nullptr)
        : StalledIssueChooseWidget(parent)
    {}

    ~CloudStalledIssueChooseWidget() = default;

    QString movedToBinText() const override;
    void updateUi(CloudStalledIssueDataPtr cloudData, LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side);
};

#endif // STALLEDISSUECHOOSEWIDGET_H
