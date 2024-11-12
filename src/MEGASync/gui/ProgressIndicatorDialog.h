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
    ~ProgressIndicatorDialog() override;

    void resetProgressBar();
    void setDialogTitle(const QString& text);
    void setDialogDescription(const QString& text);
    void setMinimumProgressBarValue(int value);
    void setMaximumProgressBarValue(int value);
    void setProgressBarValue(int value);
    int getMinimumProgressBarValue();
    int getMaximumProgressBarValue();
    int getProgressBarValue();

    void setCancelButtonEnable(bool state);

signals:
    void cancelClicked();

private:
    Ui::ProgressIndicatorDialog* ui;
};

#endif // PROGRESS_INDICATOR_DIALOG_H
