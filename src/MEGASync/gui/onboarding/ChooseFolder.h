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
    void folderChanged();

public:
    ChooseLocalFolder(QObject* parent = nullptr);
    Q_INVOKABLE void openFolderSelector();
    Q_INVOKABLE const QString getFolder();
    Q_INVOKABLE void reset();
    void createDefault();

    static QString DEFAULT_FOLDER;
    static QString DEFAULT_FOLDER_PATH;

private:
    QString mFolder;
    QString mFolderName;
};

class ChooseRemoteFolder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(mega::MegaHandle folderHandle MEMBER mFolderHandle READ getHandle NOTIFY folderNameChanged)
    Q_PROPERTY(QString folderName MEMBER mFolderName READ getFolderName NOTIFY folderNameChanged)

signals:
    void folderNameChanged();

public:
    ChooseRemoteFolder(QObject* parent = nullptr);
    Q_INVOKABLE void openFolderSelector();
    Q_INVOKABLE mega::MegaHandle getHandle();
    Q_INVOKABLE void reset();
    Q_INVOKABLE const QString getFolderName();

private:
    mega::MegaHandle mFolderHandle;
    QString mFolderName;
};

#endif // CHOOSESYNCFOLDER_H
