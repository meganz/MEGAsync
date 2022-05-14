#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QDialog>
#include <QValidator>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = nullptr);
    ~RenameDialog();

    void setMessage(const QString& message);
    void setEditorValidator(const QValidator *v);

    QString getRenameText();

private:
    Ui::RenameDialog *ui;
};

#endif // RENAMEDIALOG_H
