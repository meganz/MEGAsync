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
    static const int BUTTON_ID;

    explicit StalledIssueChooseWidget(QWidget *parent = nullptr);
    virtual ~StalledIssueChooseWidget();

    void hideActionButton();
    bool isSolved() const;

signals:
    void chooseButtonClicked(int id);

protected slots:
    virtual void onRawInfoToggled(){}

protected:
    virtual void setSolved(bool isDiscarded);
    virtual QString solvedString() const = 0;
    Ui::StalledIssueChooseWidget *ui;

private slots:
    void onActionClicked(int button_id);

private:
    bool mIsSolved;
    QPointer<QGraphicsOpacityEffect> mPathDisableEffect;
};

class GenericChooseWidget : public StalledIssueChooseWidget
{
    Q_OBJECT

public:
    explicit GenericChooseWidget(QWidget *parent = nullptr)
        : StalledIssueChooseWidget(parent)
    {}

    QString solvedString() const override;

    void setSolved(bool isDiscarded) override;

    struct GenericInfo
    {
        QString icon;
        QString title;
        QString buttonText;
        QString solvedText;
    };

    void setInfo(const GenericInfo& info);

private:
    GenericInfo mInfo;
};

#endif // STALLEDISSUECHOOSEWIDGET_H
