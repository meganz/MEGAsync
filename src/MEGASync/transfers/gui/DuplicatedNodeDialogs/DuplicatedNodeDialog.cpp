#include "DuplicatedNodeDialogs/DuplicatedNodeDialog.h"
#include "ui_DuplicatedNodeDialog.h"

#include "DuplicatedNodeItem.h"
#include "Utilities.h"

#include <QFileInfo>

DuplicatedNodeDialog::DuplicatedNodeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DuplicatedNodeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_WINDOWS
    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
#endif

    connect(&mFolderCheck, &DuplicatedUploadBase::selectionDone, this, [this](){
        done(QDialog::Accepted);
    });
    connect(&mFileCheck, &DuplicatedUploadBase::selectionDone, this, [this](){
        done(QDialog::Accepted);
    });

    QIcon warningIcon(QString::fromLatin1(":/images/icon_warning.png"));
    ui->lIcon->setPixmap(warningIcon.pixmap(ui->lIcon->size()));

    ui->lDescription->installEventFilter(this);
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
        conflict->hasConflict() ? mFileConflicts.append(conflict) : mUploads.append(conflict);
    }
    else
    {
        auto conflict = mFolderCheck.checkUpload(nodePath, parentNode);
        conflict->hasConflict() ? mFolderConflicts.append(conflict) : mUploads.append(conflict);
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
        QString checkBoxText(tr("Apply this option on the next %1 conflict", "", count).arg(count));
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
}

void DuplicatedNodeDialog::fillDialog(const QList<std::shared_ptr<DuplicatedNodeInfo> > &conflicts, DuplicatedUploadBase *checker)
{
    auto conflictNumber(conflicts.size());
    setConflictItems(conflictNumber);

    for(auto conflictIt = conflicts.begin(); conflictIt != conflicts.end(); ++conflictIt)
    {
        checker->fillUi(this, (*conflictIt));
        adjustSize();
        exec();

        if(ui->cbApplyToAll->isChecked())
        {
             for(auto it = conflictIt; it != conflicts.end(); ++it)
             {
                 (*it)->setSolution((*conflictIt)->getSolution());
                 if((*it)->getSolution() != NodeItemType::DONT_UPLOAD)
                 {
                     mUploads.append((*it));
                 }
             }

             break;
        }
        else
        {
            if((*conflictIt)->getSolution() != NodeItemType::DONT_UPLOAD)
            {
                 mUploads.append((*conflictIt));
            }
        }

        conflictNumber--;

        cleanUi();
        setConflictItems(conflictNumber);
    }
}

QList<std::shared_ptr<DuplicatedNodeInfo>> DuplicatedNodeDialog::show()
{
    //Show folders conflicts
    if(!mFolderConflicts.isEmpty())
    {
        setDialogTitle(tr("Folder already exists"));
        fillDialog(mFolderConflicts, &mFolderCheck);

        cleanUi();
    }

    //show files conflicts
    if(!mFileConflicts.isEmpty())
    {
        setDialogTitle(tr("File already exists"));
        fillDialog(mFileConflicts, &mFileCheck);
    }

    return mUploads;
}

void DuplicatedNodeDialog::setDialogTitle(const QString &title)
{
    setWindowTitle(title);
}

bool DuplicatedNodeDialog::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == ui->lDescription && event->type() == QEvent::Resize)
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
    }

    return QDialog::eventFilter(watched, event);
}
