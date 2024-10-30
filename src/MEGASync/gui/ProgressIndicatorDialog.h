#ifndef PROGRESS_INDICATOR_DIALOG_H
#define PROGRESS_INDICATOR_DIALOG_H

#include <QDialog>

namespace Ui
{
class ProgressIndicatorDialog;
}

class ProgressIndicatorDialog: public QDialog
{
    Q_OBJECT

public:
    explicit ProgressIndicatorDialog(QWidget* parent);
    ~ProgressIndicatorDialog();
    Ui::ProgressIndicatorDialog* ui;
};

#endif // PROGRESS_INDICATOR_DIALOG_H
