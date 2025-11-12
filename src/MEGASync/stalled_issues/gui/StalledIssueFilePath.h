#ifndef STALLEDISSUEFILEPATH_H
#define STALLEDISSUEFILEPATH_H

#include "StalledIssue.h"

#include <QEvent>
#include <QIcon>
#include <QLabel>
#include <QWidget>

namespace Ui {
class StalledIssueFilePath;
}

class IconLabel;

class StalledIssueFilePath : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssueFilePath(QWidget *parent = nullptr);
    ~StalledIssueFilePath();

    void setIndent(int indent);
    void updateUi(StalledIssueDataPtr newData);
    void showFullPath();
    void hideLocalOrRemoteTitle();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onHelpIconClicked();

private:
    void showHoverAction(QEvent::Type type, QWidget* actionWidget, const QString& path);
    void updateFileIcons();
    void updateMoveFileIcons();
    void updateCornerArrows();
    void updateLocalOrMegaTitle();

    void fillFilePath();
    QString getFilePath() const;

    void fillMoveFilePath();
    QString getMoveFilePath() const;

    std::unique_ptr<mega::MegaNode> getNode() const;
    std::unique_ptr<mega::MegaNode> getMoveNode() const;

    QString getSyncPathProblemString(mega::MegaSyncStall::SyncPathProblem pathProblem);
    bool showError(mega::MegaSyncStall::SyncPathProblem pathProblem);
    QUrl getHelpLink(mega::MegaSyncStall::SyncPathProblem pathProblem);

    Ui::StalledIssueFilePath *ui;
    StalledIssueDataPtr mData;
    bool mShowFullPath;
};

#endif // STALLEDISSUEFILEPATH_H
