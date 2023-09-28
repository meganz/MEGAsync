#include "AbstractPlatform.h"

#include "MultiQFileDialog.h"
#include "control/DialogOpener.h"

#include <QScreen>
#include <QDesktopWidget>

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

void AbstractPlatform::uninstall()
{
}

void AbstractPlatform::initMenu(QMenu *m, const char *objectName, const bool applyDefaultStyling)
{
    if (m)
    {
        m->setObjectName(QString::fromUtf8(objectName));
        if (applyDefaultStyling)
        {
            m->setStyleSheet(QLatin1String("QMenu {"
                                               "background: #ffffff;"
                                               "padding-top: 6px;"
                                               "padding-bottom: 6px;"
                                               "border: 1px solid #B8B8B8;"
                                           "}"
                                           "QMenu::separator {"
                                               "height: 1px;"
                                               "margin: 6px 10px 6px 10px;"
                                               "background-color: rgba(0, 0, 0, 0.1);"
                                           "}"
                                           // For vanilla QMenus (only in TransferManager and NodeSelectorTreeView (NodeSelector))
                                           "QMenu::item {"
                                               "font-family: Lato;"
                                               "font-size: 14px;"
                                               "margin: 6px 16px 6px 16px;"
                                               "color: #777777;"
                                               "padding-right: 16px;"
                                           "}"
                                           "QMenu::item:selected {"
                                               "color: #000000;"
                                           "}"
                                           // For menus with MenuItemActions
                                           "QLabel {"
                                               "font-family: Lato;"
                                               "font-size: 14px;"
                                               "padding: 0px;"
                                           "}"
                                           ));
            m->ensurePolished();
        }
    }
}

QString AbstractPlatform::getSizeStringLocalizedOSbased(qint64 bytes)
{
    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);
    return locale.formattedDataSize(bytes, 2, QLocale::DataSizeFormat::DataSizeTraditionalFormat);
}

void AbstractPlatform::fileSelector(const SelectorInfo &info)
{
    auto previousFileUploadSelector = DialogOpener::findDialog<QFileDialog>();
    QString defaultDir = info.defaultDir;
    if(previousFileUploadSelector)
    {
        defaultDir = previousFileUploadSelector->getDialog()->directory().path();
    }

    QPointer<QFileDialog> fileDialog = new QFileDialog(info.parent);
    fileDialog->setWindowTitle(info.title);
    fileDialog->setDirectory(defaultDir);
    fileDialog->setOption(QFileDialog::DontResolveSymlinks, true);
    fileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
    if(info.multiSelection)
    {
        fileDialog->setFileMode(QFileDialog::ExistingFiles);
    }
    else
    {
        fileDialog->setFileMode(QFileDialog::ExistingFile);
    }
    //Orphan native dialogs must be modal in Windows and Linux. On macOS this method has its own implementation.
    if(!info.parent)
    {
        fileDialog->setModal(true);
    }

    auto& func = info.func;
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

void AbstractPlatform::folderSelector(const SelectorInfo &info)
{
    auto previousFileUploadSelector = DialogOpener::findDialog<QFileDialog>();
    QString defaultDir = info.defaultDir;
    if(previousFileUploadSelector)
    {
        defaultDir = previousFileUploadSelector->getDialog()->directory().path();
    }

    auto& func = info.func;

    if(!info.multiSelection)
    {
        QPointer<QFileDialog> fileDialog = new QFileDialog(info.parent);
        fileDialog->setWindowTitle(info.title);
        fileDialog->setDirectory(defaultDir);
        fileDialog->setOption(QFileDialog::DontResolveSymlinks, true);
        fileDialog->setOption(QFileDialog::DontUseNativeDialog, false);
        fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        fileDialog->setFileMode(QFileDialog::DirectoryOnly);
        //Orphan native dialogs must be modal in Windows and Linux. On macOS this method has its own implementation.
        if(!info.parent)
        {
            fileDialog->setModal(true);
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
    else
    {
        auto multiUploadFileDialog = new MultiQFileDialog(info.parent,
                                                      info.title,
                                                      defaultDir, info.multiSelection);
        multiUploadFileDialog->setOption(QFileDialog::DontResolveSymlinks, true);
        multiUploadFileDialog->setOption(QFileDialog::ShowDirsOnly, true);

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

void AbstractPlatform::fileAndFolderSelector(const SelectorInfo &info)
{
    auto previousFileUploadSelector = DialogOpener::findDialog<MultiQFileDialog>();
    QString defaultDir = info.defaultDir;
     if(previousFileUploadSelector)
     {
         defaultDir = previousFileUploadSelector->getDialog()->directory().path();
     }

    auto multiUploadFileDialog = new MultiQFileDialog(info.parent,
                                                  info.title,
                                                  defaultDir, info.multiSelection);

    multiUploadFileDialog->setOption(QFileDialog::DontResolveSymlinks, true);

    auto& func = info.func;
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

void AbstractPlatform::closeFileFolderSelectors(QWidget *parent)
{

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

QString AbstractPlatform::rectToString(const QRect &rect)
{
    return QString::fromUtf8("[%1,%2,%3,%4]").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
}

void AbstractPlatform::logInfoDialogCoordinates(const char *message, const QRect &screenGeometry, const QString &otherInformation)
{
    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. %1: valid = %2, geom = %3, %4")
                       .arg(QString::fromUtf8(message))
                       .arg(screenGeometry.isValid())
                       .arg(rectToString(screenGeometry))
                       .arg(otherInformation)
                       .toUtf8().constData());
}
