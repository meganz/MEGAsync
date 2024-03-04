#ifndef STALLEDISSUEHEADER_H
#define STALLEDISSUEHEADER_H

#include "megaapi.h"
#include "StalledIssue.h"
#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QFutureWatcher>
#include <QPointer>

#include "ui_StalledIssueHeader.h"

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
    void setIsExpandable(bool newIsExpandable);

    virtual bool adaptativeHeight();

    //void showAction(const QString& actionButtonText);
    //void hideAction();

    struct ActionInfo
    {
        ActionInfo(const QString& text, int id):
            actionText(text),
            id(id)
        {}

        QString actionText;
        int id;
    };

    void updateHeaderSizes();

    void showAction(const ActionInfo& actions);
    void showActions(const QString& actionButtonText, const QList<ActionInfo>& actions);
    void hideAction();

    void showMessage(const QString& message, const QPixmap &pixmap);
    void showSolvedMessage(const QString& customMessage = QString());

    void setText(const QString& text);
    QString displayFileName(bool preferCloud = false);

    void setTitleDescriptionText(const QString& text);

    void setData(StalledIssueHeaderCase* issueData);
    void reset();

    void refreshCaseTitles();
    void refreshCaseActions();

protected:
    QString fileName();

private slots:
    void onMultipleActionClicked();

private:
    void showIgnoreFile();
    void issueIgnored();
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
