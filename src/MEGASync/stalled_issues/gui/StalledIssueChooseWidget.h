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

protected slots:
    virtual void onRawInfoToggled(){}

protected:
    virtual QString movedToBinText() const = 0;
    bool eventFilter(QObject *watched, QEvent *event) override;
    Ui::StalledIssueChooseWidget *ui;
    StalledIssueDataPtr mData;

private slots:
    void onActionClicked(int button_id);

private:
    void setSolved();

    QPointer<QGraphicsOpacityEffect> mDisableEffect;
};

class LocalStalledIssueChooseWidget : public StalledIssueChooseWidget
{
    Q_OBJECT

public:
    explicit LocalStalledIssueChooseWidget(QWidget *parent = nullptr)
        : StalledIssueChooseWidget(parent)
    {}

    ~LocalStalledIssueChooseWidget() = default;

    QString movedToBinText() const override;
    void updateUi(LocalStalledIssueDataPtr localData, LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side);

protected slots:
    void onRawInfoToggled() override;

private:
    void updateExtraInfo(LocalStalledIssueDataPtr data);
};

class CloudStalledIssueChooseWidget : public StalledIssueChooseWidget
{
    Q_OBJECT

public:
    explicit CloudStalledIssueChooseWidget(QWidget *parent = nullptr)
        : StalledIssueChooseWidget(parent)
    {}

    ~CloudStalledIssueChooseWidget() = default;

    QString movedToBinText() const override;
    void updateUi(CloudStalledIssueDataPtr cloudData, LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side);

protected slots:
    void onRawInfoToggled() override;

private:
    void updateExtraInfo(CloudStalledIssueDataPtr data);
};

#endif // STALLEDISSUECHOOSEWIDGET_H
