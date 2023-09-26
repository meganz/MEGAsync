#ifndef CHOOSEFOLDER_H
#define CHOOSEFOLDER_H

#include <QObject>

#include <megaapi.h>

class ChooseLocalFolder : public QObject
{
    Q_OBJECT

public:
    ChooseLocalFolder(QObject* parent = nullptr);
    Q_INVOKABLE void openFolderSelector(const QString& folderPath = QString());
    Q_INVOKABLE bool createFolder(const QString& folderPath);
    Q_INVOKABLE QString getDefaultFolder(const QString& folderName = QString());

signals:
    void folderChoosen(QString folder, QString folderName);
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
    Q_INVOKABLE const mega::MegaHandle getHandle();
    Q_INVOKABLE void reset();
    Q_INVOKABLE const QString getFolderName();

    static QString DEFAULT_LOCAL_FOLDER;
    static QString DEFAULT_LOCAL_FOLDER_PATH;

private:
    mega::MegaHandle mFolderHandle;
    QString mFolderName;
};

#endif // CHOOSESYNCFOLDER_H
