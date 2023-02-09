#include "AbstractPlatform.h"

#include "MultiQFileDialog.h"
#include "control/DialogOpener.h"

void AbstractPlatform::prepareForSync()
{

}

bool AbstractPlatform::enableTrayIcon(QString /*executable*/)
{
    return false;
}


bool AbstractPlatform::isTilingWindowManager()
{
    return false;
}

QByteArray AbstractPlatform::encrypt(QByteArray data, QByteArray /*key*/)
{
    return data;
}

QByteArray AbstractPlatform::decrypt(QByteArray data, QByteArray /*key*/)
{
    return data;
}

QByteArray AbstractPlatform::getLocalStorageKey()
{
    return QByteArray(128, 0);
}

QString AbstractPlatform::getDefaultFileBrowserApp()
{
    return QString();
}

QString AbstractPlatform::getDefaultOpenAppByMimeType(QString /*mimeType*/)
{
    return QString();
}

bool AbstractPlatform::getValue(const char * const /*name*/, const bool /*default_value*/)
{
    return false;
}

std::string AbstractPlatform::getValue(const char * const /*name*/, const std::string& /*default_value*/)
{
    return std::string("");
}

QString AbstractPlatform::getWindowManagerName()
{
    return QString();
}

void AbstractPlatform::enableDialogBlur(QDialog* /*dialog*/)
{
}

void AbstractPlatform::execBackgroundWindow(QDialog *window)
{
    window->exec();
}

void AbstractPlatform::showBackgroundWindow(QDialog *window)
{
    window->show();
}

void AbstractPlatform::uninstall()
{
}

void AbstractPlatform::fileSelector(QString title, QString defaultDir, bool multiSelection, QWidget* parent, std::function<void(QStringList)> func)
{
    auto previousFileUploadSelector = DialogOpener::findDialogByClass<QFileDialog>();
    if(previousFileUploadSelector)
    {
        defaultDir = previousFileUploadSelector->getDialog()->directory().path();
    }

    QPointer<QFileDialog> fileDialog = new QFileDialog(parent);
    fileDialog->setWindowTitle(title);
    fileDialog->setDirectory(defaultDir);
    fileDialog->setOption(QFileDialog::DontResolveSymlinks, true);
    fileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
    if(multiSelection)
    {
        fileDialog->setFileMode(QFileDialog::ExistingFiles);
    }
    else
    {
        fileDialog->setFileMode(QFileDialog::ExistingFile);
    }
    DialogOpener::showDialog<QFileDialog>(fileDialog, [fileDialog, func]()
    {
        QStringList files;
        if(fileDialog->result() == QDialog::Accepted)
        {
            files = fileDialog->selectedFiles();
        }
        func(files);
    });
}

void AbstractPlatform::folderSelector(QString title, QString defaultDir, bool multiSelection, QWidget* parent, std::function<void(QStringList)> func)
{
    auto previousFileUploadSelector = DialogOpener::findDialogByClass<QFileDialog>();
    if(previousFileUploadSelector)
    {
        defaultDir = previousFileUploadSelector->getDialog()->directory().path();
    }

    if(!multiSelection)
    {
        QPointer<QFileDialog> fileDialog = new QFileDialog(parent);
        fileDialog->setWindowTitle(title);
        fileDialog->setDirectory(defaultDir);
        fileDialog->setOption(QFileDialog::DontResolveSymlinks, true);
        fileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
        fileDialog->setFileMode(QFileDialog::DirectoryOnly);

        DialogOpener::showDialog<QFileDialog>(fileDialog, [fileDialog, func]()
        {
            QStringList files;
            if(fileDialog->result() == QDialog::Accepted)
            {
                files = fileDialog->selectedFiles();
            }
            func(files);
        });
    }
    else
    {
        auto multiUploadFileDialog = new MultiQFileDialog(parent,
                                                      title,
                                                      defaultDir, multiSelection);
        multiUploadFileDialog->setOption(QFileDialog::DontResolveSymlinks, true);
        multiUploadFileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
        multiUploadFileDialog->setFileMode(QFileDialog::DirectoryOnly);

        DialogOpener::showDialog<MultiQFileDialog>(multiUploadFileDialog, [multiUploadFileDialog, func](){
            QStringList files;
            if(multiUploadFileDialog->result() == QDialog::Accepted)
            {
                files = multiUploadFileDialog->selectedFiles();
            }
            func(files);
        });
    }
}

void AbstractPlatform::fileAndFolderSelector(QString title, QString defaultDir, bool multiSelection, QWidget* parent, std::function<void(QStringList)> func)
{
    auto previousFileUploadSelector = DialogOpener::findDialogByClass<MultiQFileDialog>();
     if(previousFileUploadSelector)
     {
         defaultDir = previousFileUploadSelector->getDialog()->directory().path();
     }

    auto multiUploadFileDialog = new MultiQFileDialog(parent,
                                                  title,
                                                  defaultDir, multiSelection);

    multiUploadFileDialog->setOption(QFileDialog::DontResolveSymlinks, true);

    DialogOpener::showDialog<MultiQFileDialog>(multiUploadFileDialog, [func, multiUploadFileDialog](){
        QStringList files;
        if(multiUploadFileDialog->result() == QDialog::Accepted)
        {
            files = multiUploadFileDialog->selectedFiles();
        }
        func(files);
    });
}

void AbstractPlatform::raiseFileFolderSelectors()
{
    DialogOpener::raiseAllDialogs();
}

void AbstractPlatform::addSyncToLeftPane(QString /*syncPath*/, QString /*syncName*/, QString /*uuid*/)
{
}

void AbstractPlatform::removeAllSyncsFromLeftPane()
{
}

bool AbstractPlatform::makePubliclyReadable(const QString& /*fileName*/)
{
    return false;
}

std::shared_ptr<AbstractShellNotifier> AbstractPlatform::getShellNotifier()
{
    return mShellNotifier;
}
