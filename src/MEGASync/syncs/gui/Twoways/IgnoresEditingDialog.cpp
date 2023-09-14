#include "IgnoresEditingDialog.h"
#include "ui_IgnoresEditingDialog.h"

#include "AddExclusionDialog.h"

#include <QPointer>

IgnoresEditingDialog::IgnoresEditingDialog(const QString &syncLocalFolder, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IgnoresEditingDialog),
    mPreferences(Preferences::instance()),
    mManager(syncLocalFolder)
{
    ui->setupUi(this);

    ui->cbExcludeLowerUnit->addItems(MegaIgnoreSizeRule::getUnitsForDisplay());
    auto lowLimit = mManager.getLowLimitRule();
    if(lowLimit)
    {
        ui->eLowerThan->setValue(lowLimit->value());
        ui->cbExcludeLowerUnit->setCurrentIndex(lowLimit->unit());
        ui->cExcludeLowerThan->setChecked(!lowLimit->isCommented());
        ui->eLowerThan->setEnabled(!lowLimit->isCommented());
        ui->cbExcludeLowerUnit->setEnabled(!lowLimit->isCommented());
    }
    else
    {
        ui->eLowerThan->setValue(0);
        ui->cbExcludeLowerUnit->setCurrentIndex(MegaIgnoreSizeRule::UnitTypes::B);
    }

    ui->cbExcludeUpperUnit->addItems(MegaIgnoreSizeRule::getUnitsForDisplay());
    auto highLimit = mManager.getHighLimitRule();
    if(highLimit)
    {
        ui->eUpperThan->setValue(highLimit->value());
        ui->cbExcludeUpperUnit->setCurrentIndex(highLimit->unit());
        ui->cExcludeUpperThan->setChecked(!highLimit->isCommented());
        ui->eUpperThan->setEnabled(!highLimit->isCommented());
        ui->cbExcludeUpperUnit->setEnabled(!highLimit->isCommented());
    }
    else
    {
        ui->eUpperThan->setValue(0);
        ui->cbExcludeUpperUnit->setCurrentIndex(MegaIgnoreSizeRule::UnitTypes::B);
    }

    QObject::connect(ui->cExcludeExtenstions, &QCheckBox::toggled, ui->tExcludeExtensions, &QPlainTextEdit::setEnabled);
    ui->tExcludeExtensions->setEnabled(false);
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

void IgnoresEditingDialog::on_eUpperThan_valueChanged(int i)
{
    auto highLimit = mManager.getHighLimitRule();
    highLimit->setValue(i);
}

void IgnoresEditingDialog::on_eLowerThan_valueChanged(int i)
{
    auto lowLimit = mManager.getLowLimitRule();
    lowLimit->setValue(i);
}

void IgnoresEditingDialog::on_cbExcludeUpperUnit_currentIndexChanged(int i)
{
    auto highLimit = mManager.getHighLimitRule();
    highLimit->setUnit(i);
}

void IgnoresEditingDialog::on_cbExcludeLowerUnit_currentIndexChanged(int i)
{
    auto lowLimit = mManager.getLowLimitRule();
    lowLimit->setUnit(i);
}

void IgnoresEditingDialog::on_cExcludeUpperThan_clicked()
{
    bool enable (ui->cExcludeUpperThan->isChecked());

    ui->eUpperThan->setEnabled(enable);
    ui->cbExcludeUpperUnit->setEnabled(enable);

    auto highLimit = mManager.getHighLimitRule();
    highLimit->setCommented(!enable);
}

void IgnoresEditingDialog::on_cExcludeLowerThan_clicked()
{
    bool enable (ui->cExcludeLowerThan->isChecked());

    ui->eLowerThan->setEnabled(enable);
    ui->cbExcludeLowerUnit->setEnabled(enable);

    auto lowLimit = mManager.getLowLimitRule();
    lowLimit->setCommented(!enable);
}
//
