#include "DuplicatedNodeDialog.h"

#include "EventUpdater.h"
#include "MegaApplication.h"
#include "ui_DuplicatedNodeDialog.h"
#include "WordWrapLabel.h"

#include <QFileInfo>
#include <QScreen>

QSet<int> DuplicatedNodeDialog::mIgnoreConflictTypes = QSet<int>();

DuplicatedNodeDialog::DuplicatedNodeDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DuplicatedNodeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WINDOWS
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
#elif defined Q_OS_LINUX
    layout()->setSizeConstraint(QLayout::SetFixedSize);
#endif
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    ui->lDescriptionFileExists->installEventFilter(this);

    qRegisterMetaType<QList<std::shared_ptr<DuplicatedNodeInfo>>>("QList<std::shared_ptr<DuplicatedNodeInfo>");

    mSizeAdjustTimer.setSingleShot(true);
    mSizeAdjustTimer.setInterval(0);
    connect(&mSizeAdjustTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                adjustSize();
            });
}

DuplicatedNodeDialog::~DuplicatedNodeDialog()
{
    mIgnoreConflictTypes.clear();
    delete ui;
}

void DuplicatedNodeDialog::addNodeItem(DuplicatedNodeItem* item)
{
    if (ignoreConflictType(item->getType()))
    {
        item->hide();
        item->deleteLater();
    }
    else
    {
        ui->nodeItemsLayout->addWidget(item);
    }
}

void DuplicatedNodeDialog::cleanUi()
{
    while(auto nodeItem = ui->nodeItemsLayout->takeAt(0))
    {
        if (nodeItem->widget())
        {
            delete nodeItem->widget();
        }
        delete nodeItem;
    }
}

void DuplicatedNodeDialog::setConflictItems(qsizetype count)
{
    if(count > 1)
    {
        static const qsizetype maxInt = std::numeric_limits<int>::max();
        const auto intCount = static_cast<int>(std::min(count, maxInt)); // Safe cast
        const auto checkBoxText(tr("Apply to all %1 duplicates", "", intCount).arg(count));
        ui->cbApplyToAll->setText(checkBoxText);
        ui->cbApplyToAll->show();
    }
    else
    {
        ui->cbApplyToAll->hide();
    }
}

void DuplicatedNodeDialog::setHeader(const QString& baseText, const QString& nodeName)
{
    mHeaderBaseName = baseText;
    mCurrentNodeName = nodeName;

    updateHeader();
}

void DuplicatedNodeDialog::fillDialog()
{
    const auto conflictNumber(mConflictsBeingProcessed.size());
    setConflictItems(conflictNumber);
    processConflict(mConflictsBeingProcessed.first());
}

void DuplicatedNodeDialog::updateReservedNames(std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    if (!conflict)
    {
        return;
    }

    auto parentNode = conflict->getParentNode();
    if (!parentNode || !mConflicts)
    {
        conflict->setReservedNames(QStringList());
        return;
    }

    conflict->setReservedNames(
        mConflicts->mReservedNodeNamesByTargetParent.value(parentNode->getHandle()));
}

void DuplicatedNodeDialog::updateReservedNames(
    const QList<std::shared_ptr<DuplicatedNodeInfo>>& conflictsBeingProcessed)
{
    for (const auto& conflict: conflictsBeingProcessed)
    {
        updateReservedNames(conflict);
    }
}

void DuplicatedNodeDialog::reserveResolvedName(std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    if (!conflict || !mConflicts || conflict->getSolution() != NodeItemType::UPLOAD_AND_RENAME)
    {
        return;
    }

    auto parentNode = conflict->getParentNode();
    if (!parentNode)
    {
        return;
    }

    auto& reservedNames = mConflicts->mReservedNodeNamesByTargetParent[parentNode->getHandle()];
    reservedNames.append(conflict->getNewName());
}

void DuplicatedNodeDialog::processConflict(std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    updateReservedNames(conflict);
    conflict->checker()->fillUi(this, conflict);
}

void DuplicatedNodeDialog::onConflictProcessed()
{
    if(!mConflictsBeingProcessed.isEmpty())
    {
        auto conflict = mConflictsBeingProcessed.takeFirst();

        if(conflict->getSolution() != NodeItemType::DONT_UPLOAD)
        {
            reserveResolvedName(conflict);
            mConflicts->mResolvedConflicts.append(conflict);
        }

        if(ui->cbApplyToAll->isChecked())
        {
            EventUpdater guiUpdater(mConflictsBeingProcessed.size(),20);
            auto counter(0);

            for(auto it = mConflictsBeingProcessed.begin(); it != mConflictsBeingProcessed.end(); ++it)
            {
                (*it)->setSolution(conflict->getSolution());
                if((*it)->getSolution() != NodeItemType::DONT_UPLOAD)
                {
                    reserveResolvedName(*it);
                    mConflicts->mResolvedConflicts.append((*it));
                }

                counter++;
                guiUpdater.update(counter);
            }

            //All conflicts have been solved
            mConflictsBeingProcessed.clear();
        }

        if(!mConflictsBeingProcessed.isEmpty())
        {
            cleanUi();
            setConflictItems(mConflictsBeingProcessed.size());
            processConflict(mConflictsBeingProcessed.first());
        }
        else
        {
            //show the following category
            startWithNewCategoryOfConflicts();
        }
    }
    else
    {
        //show the following category
        startWithNewCategoryOfConflicts();
    }
}

void DuplicatedNodeDialog::processFolderConflicts()
{
    if(!mConflicts->mFolderConflicts.isEmpty())
    {
        setDialogTitle(tr("Folder already exists"));
        mConflictsBeingProcessed = mConflicts->mFolderConflicts;
        mConflicts->mFolderConflicts.clear();
        mChecker = mConflicts->mFolderCheck;
        updateReservedNames(mConflictsBeingProcessed);
        fillDialog();
    }
}

void DuplicatedNodeDialog::processFileConflicts()
{
    //show files conflicts
    if(!mConflicts->mFileConflicts.isEmpty())
    {
        cleanUi();
        setDialogTitle(tr("File already exists"));
        mConflictsBeingProcessed = mConflicts->mFileConflicts;
        mConflicts->mFileConflicts.clear();
        mChecker = mConflicts->mFileCheck;
        updateReservedNames(mConflictsBeingProcessed);
        fillDialog();
    }
}

void DuplicatedNodeDialog::processFileNameConflicts()
{
    //show files conflicts
    if(!mConflicts->mFileNameConflicts.isEmpty())
    {
        cleanUi();
        setDialogTitle(tr("File already exists"));
        mConflictsBeingProcessed = mConflicts->mFileNameConflicts;
        mConflicts->mFileNameConflicts.clear();
        mChecker = mConflicts->mFileCheck;
        updateReservedNames(mConflictsBeingProcessed);
        fillDialog();
    }
}

void DuplicatedNodeDialog::processFolderNameConflicts()
{
    //show files conflicts
    if(!mConflicts->mFolderNameConflicts.isEmpty())
    {
        cleanUi();
        setDialogTitle(tr("Folder already exists"));
        mConflictsBeingProcessed = mConflicts->mFolderNameConflicts;
        mConflicts->mFolderNameConflicts.clear();
        mChecker = mConflicts->mFolderCheck;
        updateReservedNames(mConflictsBeingProcessed);
        fillDialog();
    }
}

void DuplicatedNodeDialog::startWithNewCategoryOfConflicts()
{
    if(!mConflicts->mFolderConflicts.isEmpty())
    {
        processFolderConflicts();
    }
    else if(!mConflicts->mFileConflicts.isEmpty())
    {
        processFileConflicts();
    }
    else if(!mConflicts->mFileNameConflicts.isEmpty())
    {
        processFileNameConflicts();
    }
    else if(!mConflicts->mFolderNameConflicts.isEmpty())
    {
        processFolderNameConflicts();
    }
    else
    {
        done(QDialog::Accepted);
    }
}

void DuplicatedNodeDialog::updateHeader()
{
    auto headerText(mHeaderBaseName);

    QString placeholder(QLatin1String("[A]"));
    auto textBoundingRect =
        ui->lDescriptionFileExists->fontMetrics().boundingRect(mHeaderBaseName).width();
    auto NameBoundingRect =
        ui->lDescriptionFileExists->fontMetrics().boundingRect(placeholder).width();

    //The node name goes in bold type, that´s why the font needs to be set to bold to get the correct fontMetrics
    auto boldFont = ui->lDescriptionFileExists->font();
    boldFont.setBold(true);
    QFontMetrics boldMetrics(boldFont);
    auto elidedName = boldMetrics.elidedText(
        mCurrentNodeName,
        Qt::ElideMiddle,
        (ui->lDescriptionFileExists->width() - (textBoundingRect - NameBoundingRect - 1)));
    auto boldName = QString(QLatin1String("<b>%1</b>")).arg(elidedName.toHtmlEscaped());

    headerText = headerText.replace(placeholder, boldName);

    ui->lDescriptionFileExists->setText(headerText);

    if (elidedName != mCurrentNodeName)
    {
        ui->lDescriptionFileExists->setToolTip(mCurrentNodeName.toHtmlEscaped());
    }
}

void DuplicatedNodeDialog::addIgnoreConflictTypes(NodeItemType ignoreConflictType)
{
    mIgnoreConflictTypes.insert(static_cast<int>(ignoreConflictType));
}

bool DuplicatedNodeDialog::ignoreConflictType(NodeItemType ignoreConflictType)
{
    return mIgnoreConflictTypes.contains(static_cast<int>(ignoreConflictType));
}

std::shared_ptr<ConflictTypes> DuplicatedNodeDialog::conflicts() const
{
    return mConflicts;
}

void DuplicatedNodeDialog::setConflicts(std::shared_ptr<ConflictTypes> newConflicts)
{
    mConflicts = newConflicts;
    connect(mConflicts->mFolderCheck, &DuplicatedUploadBase::selectionDone, this, [this](){
        onConflictProcessed();
    });
    connect(mConflicts->mFileCheck, &DuplicatedUploadBase::selectionDone, this, [this](){
        onConflictProcessed();
    });
    // Show folders conflicts
    startWithNewCategoryOfConflicts();
}

const QList<std::shared_ptr<DuplicatedNodeInfo> > &DuplicatedNodeDialog::getResolvedConflicts()
{
    return mConflicts->mResolvedConflicts;
}

bool DuplicatedNodeDialog::isEmpty() const
{
    return mConflicts->isConflictFree();
}

void DuplicatedNodeDialog::show()
{
    QDialog::show();
}

void DuplicatedNodeDialog::setDialogTitle(const QString &title)
{
    setWindowTitle(title);
}

bool DuplicatedNodeDialog::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui->lDescriptionFileExists && event->type() == QEvent::Resize)
    {
        updateHeader();
    }

    return QDialog::eventFilter(watched, event);
}

void DuplicatedNodeDialog::resizeEvent(QResizeEvent *)
{
    QRect targetGeo;
    if (QWidget* parent = parentWidget())
    {
        // Keep centered on the parent window as the dialog grows
        targetGeo = parent->window()->frameGeometry();
    }
    else if (auto screen = QGuiApplication::screenAt(this->pos()))
    {
        targetGeo = screen->geometry();
    }

    if (targetGeo.isValid())
    {
        QRect geo = geometry();
        geo.moveCenter(targetGeo.center());
        move(geo.topLeft());
    }
}

bool DuplicatedNodeDialog::event(QEvent *event)
{
    if(event->type() == WordWrapLabel::HeightAdapted)
    {
        if(sizeHint().height() != size().height())
        {
           //Size adjusted
           mSizeAdjustTimer.start();
        }
    }

    return QDialog::event(event);
}

void DuplicatedNodeDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        // The dialog's visible strings (header, option titles/buttons, descriptions and
        // "Learn more") are set programmatically, so retranslateUi() alone is not enough:
        // the content has to be rebuilt. Defer it to the next event loop, because deleting
        // the item widgets while the LanguageChange event is still being delivered to them
        // would be unsafe.
        QTimer::singleShot(0,
                           this,
                           [this]()
                           {
                               retranslateContent();
                           });
    }

    QDialog::changeEvent(event);
}

void DuplicatedNodeDialog::retranslateContent()
{
    if (mConflictsBeingProcessed.isEmpty())
    {
        return;
    }

    const bool showingFile = (mChecker == mConflicts->mFileCheck);
    setDialogTitle(showingFile ? tr("File already exists") : tr("Folder already exists"));
    setConflictItems(mConflictsBeingProcessed.size());

    // Rebuild the current conflict so its programmatically set strings are produced with the
    // freshly installed translations.
    cleanUi();
    processConflict(mConflictsBeingProcessed.first());
}
