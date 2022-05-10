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

    static const char* FULL_PATH;

public:
    explicit StalledIssueFilePath(QWidget *parent = nullptr);
    ~StalledIssueFilePath();

    void setIndent(int indent);
    void updateUi(QExplicitlySharedDataPointer<StalledIssueData> data);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void fillPathName(StalledIssueData::Path data, QLabel* label);
    void showHoverAction(QEvent::Type type, QWidget* actionWidget, const QString &path);

    void fillFilePath();
    void fillMoveFilePath();

    Ui::StalledIssueFilePath *ui;
    QExplicitlySharedDataPointer<StalledIssueData> mData;
};

#endif // STALLEDISSUEFILEPATH_H
