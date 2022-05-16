#include "RenameDialog.h"
#include "ui_RenameDialog.h"

#include <MegaApplication.h>
#include <mega/types.h>

#include <QPushButton>
#include <QFile>
#include <QDir>

RenameDialog::RenameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameDialog),
    mListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mRemoteHandle(0)
{
    ui->setupUi(this);

    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText(tr("Rename"));
    connect(okButton, &QPushButton::clicked, this, &RenameDialog::accept);

    QPushButton *cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    connect(cancelButton, &QPushButton::clicked, this, &RenameDialog::reject);

    setWindowFlags(Qt::Popup);
    resize(300,60);
}

RenameDialog::~RenameDialog()
{
    delete ui;
}

void RenameDialog::init(bool isCloud, const QString &originalPath)
{
    mIsCloud = isCloud;
    mOriginalPath = originalPath;

    QFileInfo info(originalPath);
    auto extensionText = info.completeSuffix();
    if(!extensionText.isEmpty())
    {
        ui->extension->setText(QString::fromLatin1(".") + info.completeSuffix());
    }
    else
    {
        ui->extension->hide();
    }

    if(info.isFile())
    {
        ui->message->setText(tr("Please, rename the file %1.").arg(info.fileName()));
    }
    else
    {
        ui->message->setText(tr("Please, rename the folder %1.").arg(info.fileName()));
    }

    ui->loadingLabel->hide();
    ui->errorMessage->hide();

    ui->leEditor->setFocus();
}

void RenameDialog::setEditorValidator(const QValidator *v)
{
    ui->leEditor->setValidator(v);
}

void RenameDialog::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_RENAME)
    {
        auto handle = request->getNodeHandle();
        if(handle == mRemoteHandle)
        {
            if (e->getErrorCode() == mega::MegaError::API_OK)
            {
                if(handle && handle == mRemoteHandle)
                {
                    emit renameFinished(newName());
                    mRemoteHandle = 0;
                    QDialog::accept();
                }
            }
            else
            {
                ui->errorMessage->show();
                ui->errorMessage->setText(QString::fromStdString(e->getErrorString()));
            }
        }
    }
}

void RenameDialog::accept()
{
   mIsCloud ? renameCloudFile() : renameLocalFile();
   ui->loadingLabel->show();

   setDisabled(true);
}

QString RenameDialog::newName()
{
    return ui->leEditor->text() + ui->extension->text();
}

void RenameDialog::renameCloudFile()
{
    auto fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(mOriginalPath.toStdString().c_str()));
    if(fileNode)
    {
        mRemoteHandle = fileNode->getHandle();
        MegaSyncApp->getMegaApi()->renameNode(fileNode, newName().toStdString().c_str(), mListener.get());
    }
}

void RenameDialog::renameLocalFile()
{
    QFile file(mOriginalPath);
    if(file.exists())
    {
        auto newFileName(newName());
        QFileInfo fileInfo(mOriginalPath);
        fileInfo.setFile(fileInfo.path(), newFileName);

        if(file.rename(QDir::toNativeSeparators(fileInfo.filePath())))
        {
            emit renameFinished(newFileName);
            QDialog::accept();
        }
    }
}
