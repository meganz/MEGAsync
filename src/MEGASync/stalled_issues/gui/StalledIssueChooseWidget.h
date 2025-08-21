#ifndef STALLEDISSUECHOOSEWIDGET_H
#define STALLEDISSUECHOOSEWIDGET_H

#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPointer>
#include <QWidget>

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

    void setActionButtonVisibility(bool state);
    void setMessage(const QString& string,
                    const QString& pixmapName,
                    const QString& tooltip = QString());
    void setFailed(bool state, const QString& tooltip = QString());

    void addDefaultButton();

    QString chosenString() const;

signals:
    void chooseButtonClicked(int id);

protected slots:
    virtual void onRawInfoToggled(){}

protected:
    virtual void setSolved(bool isSolved, bool isSelected);
    virtual QString solvedString() const = 0;
    Ui::StalledIssueChooseWidget *ui;

private slots:
    void onActionClicked(int button_id);

private:
    QPointer<QGraphicsOpacityEffect> mPathDisableEffect;
};

class GenericChooseWidget : public StalledIssueChooseWidget
{
    Q_OBJECT

public:
    explicit GenericChooseWidget(QWidget *parent = nullptr);

    QString solvedString() const override;

    void setSolved(bool isSolved, bool isSelected) override;

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
