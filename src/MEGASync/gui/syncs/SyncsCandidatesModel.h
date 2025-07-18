#ifndef SYNCS_CANDIDATES_MODEL_H
#define SYNCS_CANDIDATES_MODEL_H

#include <QAbstractItemModel>

#include <optional>

class SyncsCandidatesModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    using SyncCandidate = std::pair<std::string, std::string>;
    enum SyncsCandidadteModelRole
    {
        LOCAL_FOLDER = Qt::UserRole + 1,
        MEGA_FOLDER
    };

    explicit SyncsCandidatesModel(QObject* parent = nullptr);
    ~SyncsCandidatesModel() = default;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& index, int role) const override;
    virtual QModelIndex index(int row,
                              int column,
                              const QModelIndex& = QModelIndex()) const override;

    virtual QModelIndex parent(const QModelIndex&) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    friend class SyncsCandidatesController;

private:
    std::optional<std::vector<SyncCandidate>::iterator> exist(const QString& localSyncFolder,
                                                              const QString& megaSyncFolder);

    void reset();
    void add(const QString& localSyncFolder, const QString& megaSyncFolder);
    void remove(const QString& localSyncFolder, const QString& megaSyncFolder);
    void edit(const QString& originalLocalSyncFolder,
              const QString& originalMegaSyncFolder,
              const QString& localSyncFolder,
              const QString& megaSyncFolder);
    bool exist(const QString& pathToAdd, SyncsCandidadteModelRole syncsCandidateRole);

    std::vector<SyncCandidate> mSyncCandidates;
};

#endif
