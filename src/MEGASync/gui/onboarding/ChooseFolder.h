#ifndef CHOOSEFOLDER_H
#define CHOOSEFOLDER_H

#include <QObject>

#include <megaapi.h>

class ChooseLocalFolder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString folder MEMBER mFolder READ getFolder NOTIFY folderChanged)
    Q_PROPERTY(QString folderName MEMBER mFolderName)

signals:
    void folderChanged(const QString& folder);

public:
    ChooseLocalFolder(QObject* parent = nullptr);
    Q_INVOKABLE void openFolderSelector();
    Q_INVOKABLE const QString getFolder();

private:
    QString mFolder;
    QString mFolderName;
};

class ChooseRemoteFolder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(mega::MegaHandle folderHandle MEMBER mFolderHandle READ getHandle NOTIFY folderChanged)
    Q_PROPERTY(QString folderName MEMBER mFolderName)

signals:
    void folderChanged(const QString& folder);

public:
    ChooseRemoteFolder(QObject* parent = nullptr);
    Q_INVOKABLE void openFolderSelector();
    Q_INVOKABLE const mega::MegaHandle getHandle();

private:
    mega::MegaHandle mFolderHandle;
    QString mFolderName;
};

#endif // CHOOSESYNCFOLDER_H
