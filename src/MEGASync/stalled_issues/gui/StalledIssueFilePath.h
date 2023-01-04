#ifndef STALLEDISSUEFILEPATH_H
#define STALLEDISSUEFILEPATH_H

#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QLabel>
#include <QEvent>

namespace Ui {
class StalledIssueFilePath;
}

class StalledIssueFilePath : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssueFilePath(QWidget *parent = nullptr);
    ~StalledIssueFilePath();

    void setIndent(int indent);
    void updateUi(StalledIssueDataPtr data);
    void showFullPath();
    void hideLocalOrRemoteTitle();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onHelpIconClicked();

private:
    void showHoverAction(QEvent::Type type, QWidget* actionWidget, const QString &path);
    void updateFileIcons();
    void updateMoveFileIcons();

    QIcon getPathIcon(const QFileInfo& fileInfo, bool hasProblem);

    void fillFilePath();
    QString getFilePath();

    void fillMoveFilePath();
    QString getMoveFilePath();

    QString getSyncPathProblemString(mega::MegaSyncStall::SyncPathProblem pathProblem);
    QString getHelpLink(mega::MegaSyncStall::SyncPathProblem pathProblem);

    Ui::StalledIssueFilePath *ui;
    StalledIssueDataPtr mData;
    bool mShowFullPath;
};

#endif // STALLEDISSUEFILEPATH_H
