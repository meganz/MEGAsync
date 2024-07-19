#ifndef EXCLUSION_RULES_MODEL_H
#define EXCLUSION_RULES_MODEL_H

#include "MegaIgnoreManager.h"

#include <QAbstractListModel>

class ExclusionRulesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Qt::CheckState enabledRulesStatus READ getEnabledRulesStatus WRITE setEnabledRulesStatus NOTIFY enabledRulesStatusChanged)

public:

    enum TargetType {
        FILE = 0,
        FOLDER = 1,
        EXTENSION = 2,
    };
    Q_ENUM(TargetType)

    enum ExclusionRulesRole {
        TARGET_TYPE_ROLE = Qt::UserRole + 1,
        PROPERTY_ROLE,
        VALUE_RULE,
        RULE_COMMENTED_ROLE,
        ICON_NAME,
        TARGET_TYPE_INDEX,
        WILDCARD
    };

    explicit ExclusionRulesModel(QObject* parent = nullptr, std::shared_ptr<MegaIgnoreManager> megaIgnoreManager = nullptr);
    ~ExclusionRulesModel();

    QHash<int,QByteArray> roleNames() const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    void setEnabledRulesStatus(Qt::CheckState state, bool fromModel = false);
    void checkEnableAllStatus();
    QVariant data(const QModelIndex & index, int role = TARGET_TYPE_ROLE) const override;
    void setMegaIgnoreManager(std::shared_ptr<MegaIgnoreManager> megaIgnoreManager);
    std::shared_ptr<MegaIgnoreManager> getMegaIgnoreManager();
    ExclusionRulesModel::TargetType getTargetType(std::shared_ptr<MegaIgnoreRule> rule) const;
    QString getTargetTypeString(std::shared_ptr<MegaIgnoreRule> rule) const;
    QString getIconName(std::shared_ptr<MegaIgnoreRule> rule) const;
    QString getWildCardType(std::shared_ptr<MegaIgnoreRule> rule) const;
    MegaIgnoreNameRule::WildCardType getWildCard(std::shared_ptr<MegaIgnoreRule> rule) const;
    Q_INVOKABLE bool removeRow(int row);
    Q_INVOKABLE void addNewRule(int targetType, int wildCard, QString ruleVale);
    Q_INVOKABLE void editRule(int targetType, int wildCard, QString ruleVale, int index);
    void setFolder(const QString &folderName);
    Q_INVOKABLE void applyChanges();
    Qt::CheckState getEnabledRulesStatus() const;
    int ruleExist(int targetType, int wildCard, QString ruleVale);

signals:
    void enabledRulesStatusChanged();
    void newRuleAdded(int addedRuleIndex);

private:
    std::shared_ptr<MegaIgnoreManager> mMegaIgnoreManager;
    Qt::CheckState mEnableAllRulesState;
};


#endif // EXCLUSION_RULES_MODEL_H
