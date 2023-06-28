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

void AbstractPlatform::fileSelector(QString title, QString defaultDir, bool multiSelection, QWidget* parent, std::function<void(QStringList)> func)
{
    auto previousFileUploadSelector = DialogOpener::findDialog<QFileDialog>();
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
    //Orphan native dialogs must be modal in Windows and Linux. On macOS this method has its own implementation.
    if(!parent)
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

void AbstractPlatform::folderSelector(QString title, QString defaultDir, bool multiSelection, QWidget* parent, std::function<void(QStringList)> func)
{
    auto previousFileUploadSelector = DialogOpener::findDialog<QFileDialog>();
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
        fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        fileDialog->setFileMode(QFileDialog::DirectoryOnly);
        //Orphan native dialogs must be modal in Windows and Linux. On macOS this method has its own implementation.
        if(!parent)
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
        auto multiUploadFileDialog = new MultiQFileDialog(parent,
                                                      title,
                                                      defaultDir, multiSelection);
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

void AbstractPlatform::fileAndFolderSelector(QString title, QString defaultDir, bool multiSelection, QWidget* parent, std::function<void(QStringList)> func)
{
    auto previousFileUploadSelector = DialogOpener::findDialog<MultiQFileDialog>();
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

QString AbstractPlatform::RectToString(const QRect &rect)
{
    return QString::fromUtf8("[%1,%2,%3,%4]").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
}

void AbstractPlatform::logInfoDialogCoordinates(const char *message, const QRect &screenGeometry, const QString &otherInformation)
{
    /*
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. %1: valid = %2, geom = %3, %4")
                 .arg(QString::fromUtf8(message))
                 .arg(screenGeometry.isValid())
                 .arg(RectToString(screenGeometry))
                 .arg(otherInformation)
                 .toUtf8().constData());
                 */
}

void AbstractPlatform::calculateInfoDialogCoordinates(const QRect& rect, int *posx, int *posy)
{
    int xSign = 1;
    int ySign = 1;
    QPoint position;
    QRect screenGeometry;

    #ifdef __APPLE__
        QPoint positionTrayIcon;
        positionTrayIcon = trayIcon->geometry().topLeft();
    #endif

    position = QCursor::pos();
    QScreen* currentScreen = QGuiApplication::screenAt(position);
    if (currentScreen)
    {
        screenGeometry = currentScreen->availableGeometry();

        QString otherInfo = QString::fromUtf8("pos = [%1,%2], name = %3").arg(position.x()).arg(position.y()).arg(currentScreen->name());
        logInfoDialogCoordinates("availableGeometry", screenGeometry, otherInfo);

        if (!screenGeometry.isValid())
        {
            screenGeometry = currentScreen->geometry();
            otherInfo = QString::fromUtf8("dialog rect = %1").arg(RectToString(rect));
            logInfoDialogCoordinates("screenGeometry", screenGeometry, otherInfo);

            if (screenGeometry.isValid())
            {
                screenGeometry.setTop(28);
            }
            else
            {
                screenGeometry = rect;
                screenGeometry.setBottom(screenGeometry.bottom() + 4);
                screenGeometry.setRight(screenGeometry.right() + 4);
            }

            logInfoDialogCoordinates("screenGeometry 2", screenGeometry, otherInfo);
        }
        else
        {
            if (screenGeometry.y() < 0)
            {
                ySign = -1;
            }

            if (screenGeometry.x() < 0)
            {
                xSign = -1;
            }
        }

    #ifdef __APPLE__
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. posTrayIcon = %1")
                     .arg(QString::fromUtf8("[%1,%2]").arg(positionTrayIcon.x()).arg(positionTrayIcon.y()))
                     .toUtf8().constData());
        if (positionTrayIcon.x() || positionTrayIcon.y())
        {
            if ((positionTrayIcon.x() + rect.width() / 2) > screenGeometry.right())
            {
                *posx = screenGeometry.right() - rect.width() - 1;
            }
            else
            {
                *posx = positionTrayIcon.x() + trayIcon->geometry().width() / 2 - rect.width() / 2 - 1;
            }
        }
        else
        {
            *posx = screenGeometry.right() - rect.width() - 1;
        }
        *posy = screenGeometry.top();

        if (*posy == 0)
        {
            *posy = 22;
        }
    #else
        #ifdef WIN32
            QRect totalGeometry = QApplication::desktop()->screenGeometry();
            APPBARDATA pabd;
            pabd.cbSize = sizeof(APPBARDATA);
            pabd.hWnd = FindWindow(L"Shell_TrayWnd", NULL);
            //TODO: the following only takes into account the position of the tray for the main screen.
            //Alternatively we might want to do that according to where the taskbar is for the targetted screen.
            if (pabd.hWnd && SHAppBarMessage(ABM_GETTASKBARPOS, &pabd)
                    && pabd.rc.right != pabd.rc.left && pabd.rc.bottom != pabd.rc.top)
            {
                int size;
                switch (pabd.uEdge)
                {
                    case ABE_LEFT:
                        position = screenGeometry.bottomLeft();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.right - pabd.rc.left;
                            size = size * screenGeometry.height() / (pabd.rc.bottom - pabd.rc.top);
                            screenGeometry.setLeft(screenGeometry.left() + size);
                        }
                        break;
                    case ABE_RIGHT:
                        position = screenGeometry.bottomRight();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.right - pabd.rc.left;
                            size = size * screenGeometry.height() / (pabd.rc.bottom - pabd.rc.top);
                            screenGeometry.setRight(screenGeometry.right() - size);
                        }
                        break;
                    case ABE_TOP:
                        position = screenGeometry.topRight();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.bottom - pabd.rc.top;
                            size = size * screenGeometry.width() / (pabd.rc.right - pabd.rc.left);
                            screenGeometry.setTop(screenGeometry.top() + size);
                        }
                        break;
                    case ABE_BOTTOM:
                        position = screenGeometry.bottomRight();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.bottom - pabd.rc.top;
                            size = size * screenGeometry.width() / (pabd.rc.right - pabd.rc.left);
                            screenGeometry.setBottom(screenGeometry.bottom() - size);
                        }
                        break;
                }

                /*
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. pabd.uEdge = %1, pabd.rc = %2")
                             .arg(pabd.uEdge)
                             .arg(QString::fromUtf8("[%1,%2,%3,%4]").arg(pabd.rc.left).arg(pabd.rc.top).arg(pabd.rc.right).arg(pabd.rc.bottom))
                             .toUtf8().constData());
                */

            }
        #endif

        if (position.x() * xSign > (screenGeometry.right() / 2) * xSign)
        {
            *posx = screenGeometry.right() - rect.width() - 2;
        }
        else
        {
            *posx = screenGeometry.left() + 2;
        }

        if (position.y() * ySign > (screenGeometry.bottom() / 2) * ySign)
        {
            *posy = screenGeometry.bottom() - rect.height() - 2;
        }
        else
        {
            *posy = screenGeometry.top() + 2;
        }
    #endif
    }

    QString otherInfo = QString::fromUtf8("dialog rect = %1, posx = %2, posy = %3").arg(RectToString(rect)).arg(*posx).arg(*posy);
    logInfoDialogCoordinates("Final", screenGeometry, otherInfo);
}

