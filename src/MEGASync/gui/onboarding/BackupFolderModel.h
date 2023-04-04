#ifndef BACKUPFOLDERMODEL_H
#define BACKUPFOLDERMODEL_H

#include <QAbstractListModel>

struct BackupFolder
{
    QString folder;
    bool selected;
    QString size;

    BackupFolder();

    BackupFolder(const QString& folder,
                 bool selected = true);
};

class BackupFolderModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum BackupFolderRoles {
        FolderRole = Qt::UserRole + 1,
        SelectedRole,
        SizeRole
    };

    BackupFolderModel(QObject* parent = nullptr);

    QHash<int,QByteArray> roleNames() const override;

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

public slots:

    void insertFolder(const QString& folder);

    void setAllSelected(bool selected);

    int getNumSelectedRows() const;

signals:

    void rowSelectedChanged();

    void allRowsSelected(bool selected);

private:

    QList<BackupFolder> mBackupFolderList;
    QHash<int, QByteArray> mRoleNames;
    int mSelectedRowsTotal;

    void populateDefaultDirectoryList();

    void checkSelectedAll(bool selected);

};

#endif // BACKUPFOLDERMODEL_H
