#ifndef STALLEDISSUEHEADER_H
#define STALLEDISSUEHEADER_H

#include "StalledIssueBaseDelegateWidget.h"
#include "ui_StalledIssueHeader.h"

#include <QFutureWatcher>
#include <QPointer>
#include <QStyleOptionViewItem>
#include <QWidget>

class StalledIssueHeaderCase;

class StalledIssueHeader : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    enum ActionsId
    {
        Custom = 0,
        Ignore = Qt::UserRole
    };

    static const int BODY_INDENT;
    static const int ARROW_INDENT;
    static const int ICON_INDENT;
    static const int HEIGHT;
    static const int GROUPBOX_INDENT;
    static const int GROUPBOX_CONTENTS_INDENT;

    explicit StalledIssueHeader(QWidget *parent = nullptr);
    ~StalledIssueHeader();

    void expand(bool state) override;
    bool isExpandable() const;

    virtual bool adaptativeHeight();

    struct ActionInfo
    {
        ActionInfo(const QString& text, uint id = 0):
            actionText(text),
            id(id)
        {}

        QString actionText;
        uint id;
    };

    void updateHeaderSizes();

    void showAction(const ActionInfo& actions);
    void showActions(const QString& actionButtonText, const QList<ActionInfo>& actions);
    void hideAction();

    void showMessage(const QString& message, const QPixmap &pixmap);
    void updateIssueState();

    void setText(const QString& text, const QString& tooltip = QString());
    QString displayFileName(bool preferCloud = false);

    void setTitleDescriptionText(const QString& text);

    void setData(StalledIssueHeaderCase* issueData);
    void reset() override;

    void refreshCaseTitles();
    void refreshCaseActions();

protected:
    QString fileName();

private slots:
    void onMultipleActionClicked();

private:
    void setIsExpandable(bool newIsExpandable);
    void showIgnoreFile();
    void onIgnoreFileActionClicked();

    void propagateButtonClick();

    void refreshUi() override;
    void resetSolvingWidgets();

    Ui::StalledIssueHeader *ui;
    QPointer<StalledIssueHeaderCase> mHeaderCase;
    bool mIsExpandable;
};

Q_DECLARE_METATYPE(QList<StalledIssueHeader::ActionInfo>)

#endif // STALLEDISSUEHEADER_H
