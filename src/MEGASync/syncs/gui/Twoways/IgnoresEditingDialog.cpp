#include "IgnoresEditingDialog.h"
#include "ui_IgnoresEditingDialog.h"

#include "AddExclusionDialog.h"
#include "QMegaMessageBox.h"
#include "DialogOpener.h"

#include <QPointer>
#include <QDir>
#include <QtGlobal>

namespace
{
    constexpr char APPLY_TEXT[] = "Apply";
#ifdef Q_OS_MACOS
    constexpr char DISCARD_TEXT[] = "Discard changes";
#else
    constexpr char DISCARD_TEXT[] = "Discard";
#endif
    constexpr char MEGA_IGNORE_FILE_NAME[] = ".megaignore";
}

IgnoresEditingDialog::IgnoresEditingDialog(const QString &syncLocalFolder, bool createIfNotExist, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IgnoresEditingDialog),
    mIgnoresFileWatcher(std::make_shared<QFileSystemWatcher>(this)),
    mManager(syncLocalFolder, createIfNotExist),
    mSyncLocalFolder(syncLocalFolder)
{
    ui->setupUi(this);
    ui->bOpenMegaIgnore->setEnabled(QFile(syncLocalFolder + QDir::separator() + QString::fromUtf8(MEGA_IGNORE_FILE_NAME)).exists());
    // Fill units in file size comboBoxes
    ui->cbExcludeLowerUnit->blockSignals(true);
    ui->cbExcludeUpperUnit->blockSignals(true);
    ui->cbExcludeLowerUnit->addItems(MegaIgnoreSizeRule::getUnitsForDisplay());
    ui->cbExcludeUpperUnit->addItems(MegaIgnoreSizeRule::getUnitsForDisplay());
    ui->cbExcludeLowerUnit->blockSignals(false);
    ui->cbExcludeUpperUnit->blockSignals(false);

    // Prepare name rules list
    connect(ui->lExcludedNames->model(), &QAbstractItemModel::dataChanged, this, &IgnoresEditingDialog::onlExcludedNamesChanged);

    // Setup dialog buttons
    auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText(tr(APPLY_TEXT));
    auto cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    cancelButton->setText(tr(DISCARD_TEXT));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        this->mIgnoresFileWatcher->blockSignals(true);
        applyChanges();
        accept();
        });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // Fill Ui from megaignore
    auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(MEGA_IGNORE_FILE_NAME));
    mIgnoresFileWatcher->addPath(ignorePath);
    connect(mIgnoresFileWatcher.get(), &QFileSystemWatcher::fileChanged, this, &IgnoresEditingDialog::onFileChanged);
    refreshUI();

    connect(ui->eUpperThan, QOverload<int>::of(&QSpinBox::valueChanged), this, &IgnoresEditingDialog::onUpperThanValueChanged);
    connect(ui->eLowerThan, QOverload<int>::of(&QSpinBox::valueChanged), this, &IgnoresEditingDialog::onLowerThanValueChanged);
    connect(ui->cbExcludeUpperUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IgnoresEditingDialog::onExcludeUpperUnitCurrentIndexChanged);
    connect(ui->cbExcludeLowerUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IgnoresEditingDialog::onExcludeLowerUnitCurrentIndexChanged);
    connect(ui->cExcludeUpperThan, &QCheckBox::clicked, this, &IgnoresEditingDialog::onExcludeUpperThanClicked);
    connect(ui->cExcludeLowerThan, &QCheckBox::clicked, this, &IgnoresEditingDialog::onExcludeLowerThanClicked);
    connect(ui->tExcludeExtensions, &QPlainTextEdit::textChanged, this, [this]() {mExtensionsChanged = true; });

#ifdef Q_OS_MACOS
    ui->wExclusionsSegmentedControl->configureTableSegment();
    connect(ui->wExclusionsSegmentedControl, &QSegmentedControl::addButtonClicked, this, &IgnoresEditingDialog::onAddNameClicked);
    connect(ui->wExclusionsSegmentedControl, &QSegmentedControl::removeButtonClicked, this, &IgnoresEditingDialog::onDeleteNameClicked);
#else
    connect(ui->bAddName, &QPushButton::clicked, this, &IgnoresEditingDialog::onAddNameClicked);
    connect(ui->bDeleteName, &QPushButton::clicked, this, &IgnoresEditingDialog::onDeleteNameClicked);
    connect(ui->lExcludedNames, &QListWidget::itemSelectionChanged, this, [this]()
        {
            ui->bDeleteName->setEnabled(!ui->lExcludedNames->selectedItems().isEmpty());
        });
#endif
}

IgnoresEditingDialog::~IgnoresEditingDialog()
{
    delete ui;
}

void IgnoresEditingDialog::applyChanges()
{
    QStringList updatedExtensions = ui->tExcludeExtensions->toPlainText().split(QLatin1String(","));
    updatedExtensions.removeAll(QString());
    updatedExtensions.removeDuplicates();
    mManager.applyChanges(mExtensionsChanged, updatedExtensions);
}

void IgnoresEditingDialog::refreshUI()
{
    const auto allRules = mManager.getAllRules();
    ui->lExcludedNames->clear();
    // Step 0: Fill name exclusions
    for(const auto& rule : allRules)
    {
        if (rule->ruleType() == MegaIgnoreRule::RuleType::NAMERULE)
        {
            addNameRule(rule);
        }
    }
    // Step 1: Fill Size exclusions
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
    // Step 2: Fill Extension exclusions
    auto extensions(mManager.getExcludedExtensions());
    ui->tExcludeExtensions->clear();
    ui->tExcludeExtensions->document()->setPlainText(extensions.join(QLatin1String(", ")));
}

void IgnoresEditingDialog::onAddNameClicked()
{
    QPointer<AddExclusionDialog> addDialog = new AddExclusionDialog(mSyncLocalFolder, this);
    DialogOpener::showDialog<AddExclusionDialog>(addDialog, [addDialog, this]()
    {
        if (addDialog->result() != QDialog::Accepted)
        {
            return;
        }

        QString text = addDialog->textValue();
        if (text.isEmpty())
        {
            return;
        }

        MegaIgnoreNameRule::Target target(MegaIgnoreNameRule::Target::NONE);
        QFileInfo localItem(mSyncLocalFolder + QDir::separator() + text);
        if(localItem.exists())
        {
            target = localItem.isFile() ? MegaIgnoreNameRule::Target::f : MegaIgnoreNameRule::Target::d;
        }

        for (int i = 0; i < ui->lExcludedNames->count(); i++)
        {
            if (ui->lExcludedNames->item(i)->text() == text)
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.parent = this;
                msgInfo.title = QMegaMessageBox::warningTitle();
                msgInfo.text = tr("Rule already exists.");
                msgInfo.buttons = QMessageBox::Ok;
                QMegaMessageBox::warning(msgInfo);
                return;
            }
        }

        auto rule = mManager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE, text, target);
        addNameRule(rule);
    });
}

void IgnoresEditingDialog::onDeleteNameClicked()
{
    QList<QListWidgetItem*> selected = ui->lExcludedNames->selectedItems();
    if (selected.size() == 0)
    {
        return;
    }

    for (int i = 0; i < selected.size(); i++)
    {
        if(auto rule = selected[i]->data(Qt::UserRole).value<std::shared_ptr<MegaIgnoreRule> >())
        {
            rule->setDeleted(true);
        }

        delete selected[i];
    }
}

void IgnoresEditingDialog::onUpperThanValueChanged(int i)
{
    auto highLimit = mManager.getHighLimitRule();
    highLimit->setValue(i);
}

void IgnoresEditingDialog::onLowerThanValueChanged(int i)
{
    auto lowLimit = mManager.getLowLimitRule();
    lowLimit->setValue(i);
}

void IgnoresEditingDialog::onExcludeUpperUnitCurrentIndexChanged(int i)
{
    auto highLimit = mManager.getHighLimitRule();
    if (highLimit)
    {
        highLimit->setUnit(i);
    }
}

void IgnoresEditingDialog::onExcludeLowerUnitCurrentIndexChanged(int i)
{
    auto lowLimit = mManager.getLowLimitRule();
    if (lowLimit)
    {
        lowLimit->setUnit(i);
    }
}

void IgnoresEditingDialog::onlExcludedNamesChanged(const QModelIndex &topLeft, const QModelIndex&, const QVector<int> &roles)
{
    if(roles.size() == 1 && roles.first() == Qt::CheckStateRole)
    {
        if (auto rule = topLeft.data(Qt::UserRole).value<std::shared_ptr<MegaIgnoreRule> >())
        {
            rule->setCommented(!topLeft.data(Qt::CheckStateRole).toBool());
        }
    }
}

void IgnoresEditingDialog::onExcludeUpperThanClicked()
{
    bool enable (ui->cExcludeUpperThan->isChecked());

    ui->eUpperThan->setEnabled(enable);
    ui->cbExcludeUpperUnit->setEnabled(enable);

    auto highLimit = mManager.getHighLimitRule();
    highLimit->setCommented(!enable);
}

void IgnoresEditingDialog::onExcludeLowerThanClicked()
{
    bool enable (ui->cExcludeLowerThan->isChecked());

    ui->eLowerThan->setEnabled(enable);
    ui->cbExcludeLowerUnit->setEnabled(enable);

    auto lowLimit = mManager.getLowLimitRule();
    lowLimit->setCommented(!enable);
}

void IgnoresEditingDialog::onFileChanged(const QString& path)
{
#ifndef Q_OS_LINUX
    Q_UNUSED(path)
#endif

    if(mManager.hasChanged())
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

#ifdef Q_OS_LINUX
    //files on Linux are removed when modified (remove and create)
    //So we need to add it again to QFileSystemWatcher
    if (QFile::exists(path))
    {
        mIgnoresFileWatcher->addPath(path);
    }
#endif
}

void IgnoresEditingDialog::setOutputIgnorePath(const QString& outputPath)
{
    mManager.setOutputIgnorePath(outputPath);
}

void IgnoresEditingDialog::addNameRule(std::shared_ptr<MegaIgnoreRule> rule)
{
    // Sanity check
    if (rule->ruleType() != MegaIgnoreRule::RuleType::NAMERULE || rule->isCommented() || rule->getDisplayText().isEmpty())
    {
        return;
    }
    // Add rule to the list
    QListWidgetItem* item = new QListWidgetItem(rule->getDisplayText(), ui->lExcludedNames);
    item->setData(Qt::UserRole, QVariant::fromValue(rule));
}

void IgnoresEditingDialog::on_bOpenMegaIgnore_clicked()
{
    if(mSyncLocalFolder.isEmpty())
    {
        return;
    }
    QString ignore(mSyncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(ignore));
}
