#include "IgnoresEditingDialog.h"
#include "ui_IgnoresEditingDialog.h"

#include "AddExclusionDialog.h"
#include "QMegaMessageBox.h"

#include <QPointer>
#include <QDir>
IgnoresEditingDialog::IgnoresEditingDialog(const QString &syncLocalFolder, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IgnoresEditingDialog),
    mPreferences(Preferences::instance()),
    mIgnoresFileWatcher(std::make_shared<QFileSystemWatcher>(this)),
    mManager(syncLocalFolder)
{
    ui->setupUi(this);

    ui->cbExcludeLowerUnit->addItems(MegaIgnoreSizeRule::getUnitsForDisplay());
    connect(ui->lExcludedNames->model(), &QAbstractItemModel::dataChanged, this, &IgnoresEditingDialog::onlExcludedNamesChanged);

    ui->cbExcludeUpperUnit->addItems(MegaIgnoreSizeRule::getUnitsForDisplay());
   
    connect(ui->cExcludeExtenstions, &QCheckBox::toggled, this, &IgnoresEditingDialog::onCExtensionsChecked);

    QDialogButtonBox *buttonBox = findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
    if (buttonBox)
    {
        auto okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setText(QLatin1String("Apply"));

        auto cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
        cancelButton->setText(QLatin1String("Discard"));

        connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this](){
            applyChanges();
            accept();
        });
    }
    auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    mIgnoresFileWatcher->addPath(ignorePath);
    QObject::connect(mIgnoresFileWatcher.get(), &QFileSystemWatcher::fileChanged, this, &IgnoresEditingDialog::on_fileChanged);
    refreshUI();
}


IgnoresEditingDialog::~IgnoresEditingDialog()
{
    delete ui;
}

void IgnoresEditingDialog::applyChanges()
{
    mManager.applyChanges();
}

void IgnoresEditingDialog::refreshUI()
{
    const auto allRules = mManager.getAllRules();
    ui->lExcludedNames->clear();
    for(auto rule : allRules)
    {
        if (rule->ruleType() == MegaIgnoreRule::RuleType::NameRule || rule->ruleType() == MegaIgnoreRule::RuleType::InvalidRule)
        {
            QListWidgetItem* item = new QListWidgetItem(rule->getModifiedRule(), ui->lExcludedNames);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
            item->setCheckState(rule->isCommented() ? Qt::Unchecked : Qt::Checked); // AND initialize check state
            item->setData(Qt::UserRole, rule->originalRule());
            if (!rule->isValid())
            {
                static const auto red = QColor("red").lighter(180);
                item->setBackgroundColor(red);
            }
        }
    }
    auto lowLimit = mManager.getLowLimitRule();
    if (lowLimit)
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

    auto highLimit = mManager.getHighLimitRule();
    if (highLimit)
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
    auto extensions(mManager.getExcludedExtensions());
    ui->tExcludeExtensions->clear();
    ui->tExcludeExtensions->document()->setPlainText(extensions.join(QLatin1String(", ")));
    ui->cExcludeExtenstions->setChecked(!extensions.isEmpty());
    ui->tExcludeExtensions->setEnabled(!extensions.isEmpty());
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
            return;
        }
    }
    QListWidgetItem* item = new QListWidgetItem(QLatin1String("-:") + text, ui->lExcludedNames);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
    item->setCheckState(Qt::Checked); // AND initialize check state
    item->setData(Qt::UserRole, QLatin1String("-:") + text);
    mManager.addNameRule(MegaIgnoreNameRule::Class::Exclude, text );
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
        if(auto rule = mManager.getRuleByOriginalRule(selected[i]->data(Qt::UserRole).toString()))
        {
            rule->setDeleted(true);
        }

        delete selected[i];
    }
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

void IgnoresEditingDialog::onlExcludedNamesChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    if(roles.size() == 1 && roles.first() == Qt::CheckStateRole)
    {

    }
}

void IgnoresEditingDialog::onCExtensionsChecked(bool state)
{
    ui->tExcludeExtensions->setEnabled(state);
    mManager.enableExtensions(state);
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

void IgnoresEditingDialog::on_fileChanged(const QString file)
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = this;
    msgInfo.title = tr("Reload");
    msgInfo.text = tr("Current file has been modified by another program. it will be reloaded");
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok;
    QMegaMessageBox::warning(msgInfo);
    mManager.parseIgnoresFile();
    refreshUI();
}