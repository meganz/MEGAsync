#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"

#include "mega/types.h"

#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent), mega::MegaRequestListener(),
    ui(new Ui::LocalAndRemoteDifferentWidget),
    mListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mRemovedRemoteHandle(0)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

    auto margins = ui->verticalLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::ARROW_INDENT);
    ui->verticalLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::ICON_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    auto issue = getData();

    if(issue.getLocalData())
    {
        ui->chooseLocalCopy->setData(issue.getLocalData());
    }

    if(issue.getCloudData())
    {
        ui->chooseRemoteCopy->setData(issue.getCloudData());
    }
}

void LocalAndRemoteDifferentWidget::onRequestFinish(mega::MegaApi *, mega::MegaRequest *request, mega::MegaError *e)
{
    if (request->getType() == mega::MegaRequest::TYPE_MOVE)
    {
        if (e->getErrorCode() == mega::MegaError::API_OK)
        {
            auto handle = request->getNodeHandle();
            if(handle && handle == mRemovedRemoteHandle)
            {
                emit issueFixed();
                mRemovedRemoteHandle = 0;
            }
        }
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked()
{ 
    auto fileNode(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toStdString().c_str()));
    if(fileNode)
    {
        mRemovedRemoteHandle = fileNode->getHandle();
        auto rubbishNode = MegaSyncApp->getMegaApi()->getRubbishNode();
        MegaSyncApp->getMegaApi()->moveNode(fileNode,rubbishNode, mListener.get());
    }
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked()
{
    QFile file(ui->chooseLocalCopy->data()->getNativeFilePath());
    if(file.exists())
    {
#ifdef Q_OS_WIN
    // we need the "display name" of the file, so can't use nativeAbsoluteFilePath
    const QString sourcePath = QDir::toNativeSeparators(ui->chooseLocalCopy->data()->getNativeFilePath());
        // double null termination needed, so can't use QString::utf16
        QVarLengthArray<wchar_t, MAX_PATH + 1> winFile(sourcePath.length() + 2);
        sourcePath.toWCharArray(winFile.data());
        winFile[sourcePath.length()] = wchar_t{};
        winFile[sourcePath.length() + 1] = wchar_t{};

        SHFILEOPSTRUCTW operation;
        operation.hwnd = nullptr;
        operation.wFunc = FO_DELETE;
        operation.pFrom = winFile.constData();
        operation.pTo = nullptr;
        operation.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
        operation.fAnyOperationsAborted = FALSE;
        operation.hNameMappings = nullptr;
        operation.lpszProgressTitle = nullptr;

        SHFileOperation(&operation);
#elif Q_OS_MACOS // NOT TESTED
           //QMacAutoReleasePool pool;

           QFileInfo info(source.filePath());
           NSString *filepath = info.filePath().toNSString();
           NSURL *fileurl = [NSURL fileURLWithPath:filepath isDirectory:info.isDir()];
           NSURL *resultingUrl = nil;
           NSError *nserror = nil;
           NSFileManager *fm = [NSFileManager defaultManager];
           if ([fm trashItemAtURL:fileurl resultingItemURL:&resultingUrl error:&nserror] != YES) {
               error = QSystemError(nserror.code, QSystemError::NativeError);
               return false;
           }
           return true;
#elif Q_OS_UNIX //NOT TESTED
           const QFileInfo sourceInfo(ui->chooseLocalCopy->data()->getNativeFilePath());
           const QString sourcePath = sourceInfo.absolutePath();

           QDir trashDir(freeDesktopTrashLocation(sourcePath));
           if (!trashDir.exists())
           {
               return false;
           }
           const QLatin1String filesDir("files");
           const QLatin1String infoDir("info");
           trashDir.mkdir(filesDir);
           int savedErrno = errno;
           trashDir.mkdir(infoDir);
           if (!savedErrno)
               savedErrno = errno;
           if (!trashDir.exists(filesDir) || !trashDir.exists(infoDir))
           {
               return false;
           }

           const QString trashedName = sourceInfo.isDir()
                                     ? QDir(sourcePath).dirName()
                                     : sourceInfo.fileName();
           QString uniqueTrashedName = QLatin1Char('/') + trashedName;
           QString infoFileName;
           int counter = 0;
           QFile infoFile;
           auto makeUniqueTrashedName = [trashedName, &counter]() -> QString {
               ++counter;
               return QString(QLatin1String("/%1-%2"))
                                               .arg(trashedName)
                                               .arg(counter, 4, 10, QLatin1Char('0'));
           };
           do {
               while (QFile::exists(trashDir.filePath(filesDir) + uniqueTrashedName))
                   uniqueTrashedName = makeUniqueTrashedName();
               infoFileName = trashDir.filePath(infoDir)
                            + uniqueTrashedName + QLatin1String(".trashinfo");
               infoFile.setFileName(infoFileName);
               if (!infoFile.open(QIODevice::NewOnly | QIODevice::WriteOnly | QIODevice::Text))
                   uniqueTrashedName = makeUniqueTrashedName();
           } while (!infoFile.isOpen());

           const QString targetPath = trashDir.filePath(filesDir) + uniqueTrashedName;
           const QFileSystemEntry target(targetPath);

           /*
               We might fail to rename if source and target are on different file systems.
               In that case, we don't try further, i.e. copying and removing the original
               is usually not what the user would expect to happen.
           */
           if (!renameFile(source, target, error)) {
               infoFile.close();
               infoFile.remove();
               return false;
           }

           QTextStream out(&infoFile);

       #if QT_CONFIG(textcodec)
           out.setCodec("UTF-8");
       #endif
           out << "[Trash Info]" << Qt::endl;
           out << "Path=" << sourcePath << Qt::endl;
           out << "DeletionDate="
               << QDateTime::currentDateTime().toString(QLatin1String("yyyy-MM-ddThh:mm:ss")) << Qt::endl;
           infoFile.close();
#endif

         emit issueFixed();
    }
}
