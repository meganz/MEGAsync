#ifndef IGNORESEDITINGDIALOG_H
#define IGNORESEDITINGDIALOG_H

#include <QDialog>
#include <memory>
#include "Preferences.h"

namespace Ui {
class IgnoresEditingDialog;
}

class IgnoresEditingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IgnoresEditingDialog(QWidget *parent = nullptr);
    ~IgnoresEditingDialog();

public slots:
	void on_bAddName_clicked();
	void on_bDeleteName_clicked();
    void on_cExcludeUpperThan_clicked();
    void on_cExcludeLowerThan_clicked();

private:
    Ui::IgnoresEditingDialog *ui;

private:
    std::shared_ptr<Preferences> mPreferences;

};

#endif // IGNORESEDITINGDIALOG_H
