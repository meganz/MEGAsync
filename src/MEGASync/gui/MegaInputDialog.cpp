#include "MegaInputDialog.h"

#include "ThemeManager.h"
#include "ui_MegaInputDialog.h"

MegaInputDialog::MegaInputDialog(QWidget* parent):
    QDialog(parent),
    ui(new Ui::MegaInputDialog)
{
    ui->setupUi(this);
}

MegaInputDialog::~MegaInputDialog()
{
    delete ui;
}

void MegaInputDialog::setTextValue(const QString& text)
{
    ui->leMegaLink->setText(text);
}

QString MegaInputDialog::textValue() const
{
    return ui->leMegaLink->text();
}

void MegaInputDialog::setLabelText(const QString& text)
{
    ui->lDesc->setText(text);
}

QString MegaInputDialog::labelText() const
{
    return ui->lDesc->text();
}

bool MegaInputDialog::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange || event->type() == ThemeManager::ThemeChanged)
    {
        ui->retranslateUi(this);
    }
    return QDialog::event(event);
}
