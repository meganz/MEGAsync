#include "IgnoresEditingDialog.h"
#include "ui_IgnoresEditingDialog.h"

#include "AddExclusionDialog.h"
#include <QPointer>


IgnoresEditingDialog::IgnoresEditingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IgnoresEditingDialog),
    mPreferences(Preferences::instance())
{
    ui->setupUi(this);
    for (auto cb : { ui->cbExcludeUpperUnit, ui->cbExcludeLowerUnit })
    {
        cb->clear();
        cb->addItem(tr("B"));
        cb->addItem(tr("KB"));
        cb->addItem(tr("MB"));
        cb->addItem(tr("GB"));
    }
    ui->eLowerThan->setValue(static_cast<int>(mPreferences->lowerSizeLimitValue()));
    ui->cbExcludeLowerUnit->setCurrentIndex(mPreferences->lowerSizeLimitUnit());
    ui->eUpperThan->setValue(static_cast<int>(mPreferences->upperSizeLimitValue()));
    ui->cbExcludeUpperUnit->setCurrentIndex(mPreferences->upperSizeLimitUnit());
}

IgnoresEditingDialog::~IgnoresEditingDialog()
{
    delete ui;
}

void IgnoresEditingDialog::on_bAddName_clicked()
{
    QPointer<AddExclusionDialog> add = new AddExclusionDialog(this);
    int result = add->exec();
    if (!add || (result != QDialog::Accepted))
    {
        delete add;
        return;
    }

    QString text = add->textValue();
    delete add;

    if (text.isEmpty())
    {
        return;
    }

    for (int i = 0; i < ui->lExcludedNames->count(); i++)
    {
        if (ui->lExcludedNames->item(i)->text() == text)
        {
            return;
        }
        else if (ui->lExcludedNames->item(i)->text().compare(text, Qt::CaseInsensitive) > 0)
        {
            ui->lExcludedNames->insertItem(i, text);
            //saveExcludeSyncNames();
            return;
        }
    }

    ui->lExcludedNames->addItem(text);
    //saveExcludeSyncNames();
}

void IgnoresEditingDialog::on_bDeleteName_clicked()
{
    QList<QListWidgetItem*> selected = ui->lExcludedNames->selectedItems();
    if (selected.size() == 0)
    {
        return;
    }

    for (int i = 0; i < selected.size(); i++)
    {
        delete selected[i];
    }

    //saveExcludeSyncNames();
}

void IgnoresEditingDialog::on_cExcludeUpperThan_clicked()
{
    //if (mLoadingSettings) return;
    bool enable (ui->cExcludeUpperThan->isChecked());
    mPreferences->setUpperSizeLimit(enable);
    mPreferences->setCrashed(true);
    ui->eUpperThan->setEnabled(enable);
    ui->cbExcludeUpperUnit->setEnabled(enable);
    //ui->gExcludedFilesInfo->show();
    //ui->bRestart->show();
}

void IgnoresEditingDialog::on_cExcludeLowerThan_clicked()
{
    //if (mLoadingSettings) return;
    bool enable (ui->cExcludeLowerThan->isChecked());
    mPreferences->setLowerSizeLimit(enable);
    mPreferences->setCrashed(true);
    ui->eLowerThan->setEnabled(enable);
    ui->cbExcludeLowerUnit->setEnabled(enable);
    //ui->gExcludedFilesInfo->show();
    //ui->bRestart->show();
}
//