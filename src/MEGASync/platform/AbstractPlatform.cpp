#include "AbstractPlatform.h"

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

QStringList AbstractPlatform::multipleUpload(QString /*uploadTitle*/)
{
    return QStringList();
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
