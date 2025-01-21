#ifndef BACKUPCANDIDATESFOLDERSIZEREQUESTER_H
#define BACKUPCANDIDATESFOLDERSIZEREQUESTER_H

#include <QHash>
#include <QObject>
#include <QPointer>

class LocalFileFolderAttributes;

class BackupCandidatesFolderSizeRequester: public QObject
{
    Q_OBJECT

public:
    BackupCandidatesFolderSizeRequester(QObject* parent);
    ~BackupCandidatesFolderSizeRequester();

    void addFolder(const QString& folder);
    void calculateFolderSize(const QString& folder);

    void removeFolder(const QString& folder);

signals:
    void sizeReceived(QString folder, long long size);

private:
    QHash<QString, QPointer<LocalFileFolderAttributes>> mAttributesByFolder;

    void requestSize(QPointer<LocalFileFolderAttributes> attributeRequester);
};

#endif // BACKUPCANDIDATESFOLDERSIZEREQUESTER_H
