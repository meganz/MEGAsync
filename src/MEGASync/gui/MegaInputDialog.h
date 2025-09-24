#ifndef MEGAINPUTDIALOG_H
#define MEGAINPUTDIALOG_H

#include <QDialog>

namespace Ui
{
class MegaInputDialog;
}

class MegaInputDialog: public QDialog
{
    Q_OBJECT

public:
    explicit MegaInputDialog(QWidget* parent = nullptr);
    ~MegaInputDialog();

    void setTextValue(const QString& text);
    QString textValue() const;

    void setLabelText(const QString& text);
    QString labelText() const;

private:
    Ui::MegaInputDialog* ui;
    QString descText;
    QString inputText;

protected:
    bool event(QEvent* event) override;
};

#endif // MEGAINPUTDIALOG_H
