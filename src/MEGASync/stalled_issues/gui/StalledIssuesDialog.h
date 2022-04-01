#ifndef STALLEDISSUESDIALOG_H
#define STALLEDISSUESDIALOG_H

#include <QWidget>

namespace Ui {
class StalledIssuesDialog;
}

class StalledIssuesDialog : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssuesDialog(QWidget *parent = nullptr);
    ~StalledIssuesDialog();

private:
    Ui::StalledIssuesDialog *ui;
};

#endif // STALLEDISSUESDIALOG_H
