#include "DuplicatedNodeDialogs/DuplicatedNodeDialog.h"
#include "ui_DuplicatedNodeDialog.h"

#include "DuplicatedNodeItem.h"
#include "Utilities.h"
#include "EventUpdater.h"

#include <QFileInfo>

DuplicatedNodeDialog::DuplicatedNodeDialog(std::shared_ptr<mega::MegaNode> node) :
    mNode(node),
    QDialog(nullptr),
    ui(new Ui::DuplicatedNodeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WINDOWS
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
#endif
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowStaysOnTopHint);

    connect(&mFolderCheck, &DuplicatedUploadBase::selectionDone, this, [this](){
        onConflictProcessed();
    });
    connect(&mFileCheck, &DuplicatedUploadBase::selectionDone, this, [this](){
        onConflictProcessed();
    });

    QIcon warningIcon(QString::fromLatin1(":/images/icon_warning.png"));
    ui->lIcon->setPixmap(warningIcon.pixmap(ui->lIcon->size()));

    ui->lDescription->installEventFilter(this);

    qRegisterMetaType<QList<std::shared_ptr<DuplicatedNodeInfo>>>("QList<std::shared_ptr<DuplicatedNodeInfo>");
}

DuplicatedNodeDialog::~DuplicatedNodeDialog()
{
    delete ui;
}

void DuplicatedNodeDialog::checkUpload(const QString &nodePath, std::shared_ptr<mega::MegaNode> parentNode)
{
    QFileInfo fileInfo(nodePath);
    if(fileInfo.isFile())
    {
        auto conflict = mFileCheck.checkUpload(nodePath, parentNode);
        conflict->hasConflict() ? mFileConflicts.append(conflict) : mResolvedUploads.append(conflict);
    }
    else
    {
        auto conflict = mFolderCheck.checkUpload(nodePath, parentNode);
        conflict->hasConflict() ? mFolderConflicts.append(conflict) : mResolvedUploads.append(conflict);
    }
}

void DuplicatedNodeDialog::addNodeItem(DuplicatedNodeItem* item)
{
    ui->nodeItemsLayout->addWidget(item);
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

void DuplicatedNodeDialog::setConflictItems(int count)
{
    if(count > 1)
    {
        QString checkBoxText(tr("Apply to all %1 duplicates", "", count).arg(count));
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
    auto conflictNumber(mConflictsBeingProcessed.size());
    setConflictItems(conflictNumber);
    processConflict(mConflictsBeingProcessed.first());
}

void DuplicatedNodeDialog::processConflict(std::shared_ptr<DuplicatedNodeInfo> conflict)
{
    mChecker->fillUi(this, conflict);
    adjustSize();
}

void DuplicatedNodeDialog::onConflictProcessed()
{
    if(!mConflictsBeingProcessed.isEmpty())
    {
        auto conflict = mConflictsBeingProcessed.takeFirst();

        if(conflict->getSolution() != NodeItemType::DONT_UPLOAD)
        {
            mResolvedUploads.append(conflict);
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
                    mResolvedUploads.append((*it));
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
    if(!mFolderConflicts.isEmpty())
    {
        setDialogTitle(tr("Folder already exists"));
        mConflictsBeingProcessed = mFolderConflicts;
        mFolderConflicts.clear();
        mChecker = &mFolderCheck;
        fillDialog();
    }
}

void DuplicatedNodeDialog::processFileConflicts()
{
    //show files conflicts
    if(!mFileConflicts.isEmpty())
    {
        cleanUi();
        setDialogTitle(tr("File already exists"));
        mConflictsBeingProcessed = mFileConflicts;
        mFileConflicts.clear();
        mChecker = &mFileCheck;
        fillDialog();
    }
}

void DuplicatedNodeDialog::startWithNewCategoryOfConflicts()
{
    if(!mFolderConflicts.isEmpty())
    {
        processFolderConflicts();
    }
    else if(!mFileConflicts.isEmpty())
    {
        processFileConflicts();
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
    auto textBoundingRect = ui->lDescription->fontMetrics().boundingRect(mHeaderBaseName).width();
    auto NameBoundingRect = ui->lDescription->fontMetrics().boundingRect(placeholder).width();

    //The node name goes in bold type, that´s why the font needs to be set to bold to get the correct fontMetrics
    auto boldFont = ui->lDescription->font();
    boldFont.setBold(true);
    QFontMetrics boldMetrics(boldFont);
    auto elidedName = boldMetrics.elidedText(mCurrentNodeName, Qt::ElideMiddle, (ui->lDescription->width() - (textBoundingRect - NameBoundingRect - 1)));
    auto boldName = QString(QLatin1Literal("<b>%1</b>")).arg(elidedName);

    headerText = headerText.replace(placeholder, boldName);

    ui->lDescription->setText(headerText);

    if (elidedName != mCurrentNodeName)
    {
        ui->lDescription->setToolTip(mCurrentNodeName);
    }
}

const std::shared_ptr<mega::MegaNode>& DuplicatedNodeDialog::getNode() const
{
    return mNode;
}

const QList<std::shared_ptr<DuplicatedNodeInfo> > &DuplicatedNodeDialog::getResolvedConflicts()
{
    return mResolvedUploads;
}

void DuplicatedNodeDialog::show()
{
    //Show folders conflicts
    startWithNewCategoryOfConflicts();
    QDialog::show();
}

void DuplicatedNodeDialog::setDialogTitle(const QString &title)
{
    setWindowTitle(title);
}

bool DuplicatedNodeDialog::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == ui->lDescription && event->type() == QEvent::Resize)
    {
        updateHeader();
    }

    return QDialog::eventFilter(watched, event);
}
