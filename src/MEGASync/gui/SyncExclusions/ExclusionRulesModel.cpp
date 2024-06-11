#include "ExclusionRulesModel.h"

#include <QCoreApplication>

ExclusionRulesModel::ExclusionRulesModel(QObject* parent, std::shared_ptr<MegaIgnoreManager> megaIgnoreManager)
    : QAbstractListModel(parent)
{
}

ExclusionRulesModel::~ExclusionRulesModel()
{
}

QHash<int, QByteArray> ExclusionRulesModel::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {TARGET_TYPE_ROLE, "type"},
        {PROPERTY_ROLE, "property"},
        {VALUE_RULE, "value"},
        {RULE_COMMENTED_ROLE, "commented"},
        {ICON_NAME, "iconName"},
        {TARGET_TYPE_INDEX, "targetTypeIndex"},
        {WILDCARD, "wildcard"}

    };
    return roles;
}

int ExclusionRulesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() || (!mMegaIgnoreManager) ? 0 : mMegaIgnoreManager->getNameRulesCount();
}

bool ExclusionRulesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = hasIndex(index.row(), index.column(), index.parent()) && value.isValid();
    if (result)
    {
        auto rule = mMegaIgnoreManager->getNameRule(index.row());
        if (!rule)
        {
            return false;
        }
        if (role == RULE_COMMENTED_ROLE)
        {
            rule->setCommented(value.toBool());
            checkEnableAllStatus();
            emit dataChanged(index, index, { role });
        }
    }
    return result;
}

void ExclusionRulesModel::setEnabledRulesStatus(Qt::CheckState state, bool fromModel)
{
    if(!mMegaIgnoreManager)
    {
        return;
    }
    if(!fromModel && mEnableAllRulesState == Qt::CheckState::Unchecked && state == Qt::CheckState::PartiallyChecked)
    {
        state = Qt::CheckState::Checked;
    }

    if(mEnableAllRulesState == state)
    {
        return;
    }

    if(mEnableAllRulesState != Qt::CheckState::Checked && state == Qt::CheckState::Checked)
    {
        mMegaIgnoreManager->enableAllNameRules(true);
        QModelIndex topLeft = index(0, 0);
        QModelIndex bottomRight = index(rowCount() - 1, 0);
        emit dataChanged(topLeft, bottomRight);
    }
    else if(mEnableAllRulesState != Qt::CheckState::Unchecked && state == Qt::CheckState::Unchecked)
    {
        mMegaIgnoreManager->enableAllNameRules(false);
        QModelIndex topLeft = index(0, 0);
        QModelIndex bottomRight = index(rowCount() - 1, 0);
        emit dataChanged(topLeft, bottomRight);
    }
    mEnableAllRulesState = state;
    emit enabledRulesStatusChanged();
}

void ExclusionRulesModel::checkEnableAllStatus()
{
    const auto nEnabledRules = mMegaIgnoreManager->enabledRulesCount();
    auto state = Qt::PartiallyChecked;
    if(nEnabledRules == 0)
    {
        state =  Qt::Unchecked;
    }
    else if(nEnabledRules == mMegaIgnoreManager->getNameRulesCount())
    {
        state =  Qt::Checked;
    }
    setEnabledRulesStatus(state, true);
}

QVariant ExclusionRulesModel::data(const QModelIndex &index, int role) const
{
    QVariant field;
    if (!index.isValid() || index.row() >= rowCount())
    {
        return field;
    }
    if (const auto rule = mMegaIgnoreManager->getNameRule(index.row()))
    {
        switch (role) {
        case ExclusionRulesRole::TARGET_TYPE_ROLE:
            field = getTargetTypeString(rule);
            break;
        case ExclusionRulesRole::VALUE_RULE:
            field = rule->getDisplayText();
            break;
        case ExclusionRulesRole::PROPERTY_ROLE:
            field = getWildCardType(rule);
            break;
        case ExclusionRulesModel::RULE_COMMENTED_ROLE:
            field = rule->isCommented();
            break;
        case ExclusionRulesModel::ICON_NAME:
            field = getIconName(rule);
            break;
        case ExclusionRulesModel::TARGET_TYPE_INDEX:
            field = getTargetType(rule);
            break;
        case ExclusionRulesModel::WILDCARD:
            field = getWildCard(rule);
            break;
        }
    }
    return field;
}

void ExclusionRulesModel::setMegaIgnoreManager(std::shared_ptr<MegaIgnoreManager> megaIgnoreManager)
{
    beginResetModel();
    mMegaIgnoreManager = megaIgnoreManager;
    endResetModel();
}
std::shared_ptr<MegaIgnoreManager> ExclusionRulesModel::getMegaIgnoreManager()
{
    return mMegaIgnoreManager;
}

QString ExclusionRulesModel::getTargetTypeString(std::shared_ptr<MegaIgnoreRule> rule) const
{
    const auto targetType = getTargetType(rule);
    switch (targetType) {
    case ExclusionRulesModel::TargetType::EXTENSION:
        return QCoreApplication::translate("ExclusionsStrings", "file extension");
    case ExclusionRulesModel::TargetType::FILE:
        return QCoreApplication::translate("ExclusionsStrings", "file name");
    case ExclusionRulesModel::TargetType::FOLDER:
        return QCoreApplication::translate("ExclusionsStrings", "folder name");
    default:
        break;
    }
    return QString();
}

ExclusionRulesModel::TargetType ExclusionRulesModel::getTargetType(std::shared_ptr<MegaIgnoreRule> rule) const
{
    if (rule->ruleType() == MegaIgnoreRule::EXTENSIONRULE)
    {
        return ExclusionRulesModel::TargetType::EXTENSION;
    }
    else if (rule->ruleType() == MegaIgnoreRule::NAMERULE)
    {
        if (auto nameRule = std::dynamic_pointer_cast<MegaIgnoreNameRule>(rule))
        {
            const auto target = nameRule->getTarget();
            if (target == MegaIgnoreNameRule::Target::f)
            {
                return ExclusionRulesModel::TargetType::FILE;
            }
            else if (target == MegaIgnoreNameRule::Target::d)
            {
                return ExclusionRulesModel::TargetType::FOLDER;
            }
            else if (rule->getDisplayText().contains(QString::fromUtf8(".")))
            {
                return ExclusionRulesModel::TargetType::FILE;
            }
            else
            {
                return ExclusionRulesModel::TargetType::FOLDER;
            }
        }
    }
    return ExclusionRulesModel::TargetType::FILE;
}

QString ExclusionRulesModel::getIconName(std::shared_ptr<MegaIgnoreRule> rule) const
{
    if (rule->ruleType() == MegaIgnoreRule::EXTENSIONRULE)
    {
        return QString::fromUtf8("extension");
    }
    else if (rule->ruleType() == MegaIgnoreRule::NAMERULE)
    {
        if (auto nameRule = std::dynamic_pointer_cast<MegaIgnoreNameRule>(rule))
        {
            const auto target = nameRule->getTarget();
            if (target == MegaIgnoreNameRule::Target::f)
            {
                return QString::fromUtf8("file");
            }
            else if (target == MegaIgnoreNameRule::Target::d)
            {
                return QString::fromUtf8("folder");
            }
            else if (rule->getDisplayText().contains(QString::fromUtf8(".")))
            {
                return QString::fromUtf8("file");
            }
            else
            {
                return QString::fromUtf8("folder");
            }
        }
    }
    return QString();
}

QString ExclusionRulesModel::getWildCardType(std::shared_ptr<MegaIgnoreRule> rule) const
{
    if (rule->ruleType() == MegaIgnoreRule::EXTENSIONRULE)
    {
        return QCoreApplication::translate("ExclusionsStrings", "file type");
    }
    else if (rule->ruleType() == MegaIgnoreRule::NAMERULE)
    {
        if (auto nameRule = std::dynamic_pointer_cast<MegaIgnoreNameRule>(rule))
        {
            const auto wildCard = nameRule->getWildCardType();
            switch (wildCard)
            {
            case MegaIgnoreNameRule::WildCardType::CONTAINS:
                return QCoreApplication::translate("ExclusionsStrings", "contains");
            case MegaIgnoreNameRule::WildCardType::ENDSWITH:
                return QCoreApplication::translate("ExclusionsStrings", "ends with");
            case MegaIgnoreNameRule::WildCardType::STARTSWITH:
                return QCoreApplication::translate("ExclusionsStrings", "begins with");
            case MegaIgnoreNameRule::WildCardType::EQUAL:
                return QCoreApplication::translate("ExclusionsStrings", "is equal");
            default:
                break;
            } 
        }
    }
    return QString();
}

MegaIgnoreNameRule::WildCardType ExclusionRulesModel::getWildCard(std::shared_ptr<MegaIgnoreRule> rule) const
{
    if (rule->ruleType() != MegaIgnoreRule::EXTENSIONRULE)
    {
        if (auto nameRule = std::dynamic_pointer_cast<MegaIgnoreNameRule>(rule))
        {
           return nameRule->getWildCardType();
        }
    }
    //Extension rule
    return MegaIgnoreNameRule::WildCardType::ENDSWITH;
}


bool ExclusionRulesModel::removeRow(int row)
{
    if (row < 0 || row >= rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    if (const auto rule = mMegaIgnoreManager->getNameRule(row))
    {
        mMegaIgnoreManager->removeRule(rule);
    }
    checkEnableAllStatus();
    endRemoveRows();
    return true;
}

void ExclusionRulesModel::editRule(int targetType, int wildCard, QString ruleVale, int index)
{
    if(index == -1 || !mMegaIgnoreManager)
    {
        return;
    }
    auto rule = std::dynamic_pointer_cast<MegaIgnoreNameRule>(
        mMegaIgnoreManager->getNameRule(index));
    // This is to manage the case when new rules are added when editing an old rule
    auto splitted = ruleVale.split(QString::fromUtf8(","));
    if(splitted.empty())
    {
        return;
    }
    if(!rule)
    {
        return;
    }
    // Switching from name rule to extension rule
    if(rule->ruleType() != MegaIgnoreRule::RuleType::EXTENSIONRULE && targetType == TargetType::EXTENSION)
    {
        removeRow(index);
        addNewRule(targetType, wildCard, splitted[0]);
    }
    else
    {
        // We should not change target of rules targeted to soft links, as we are not supporting this target on UI
        if (rule->getTarget() != MegaIgnoreNameRule::Target::s)
        {
            rule->setTarget((targetType != TargetType::FOLDER) ? (MegaIgnoreNameRule::Target::f)
                                                               : (MegaIgnoreNameRule::Target::d));
        }
        rule->setWildCardType(static_cast<MegaIgnoreNameRule::WildCardType>(wildCard));
        rule->setPattern(splitted[0]);
    }
    emit dataChanged(createIndex(index, 0), createIndex(index, 0));
    // Remove the first value and add the rest
    splitted.pop_front();
    for(const auto& value: splitted)
    {
        addNewRule(targetType, wildCard, value);
    }
}

void ExclusionRulesModel::addNewRule(int targetType, int wildCard, QString ruleVale)
{
    if(ruleVale.trimmed().isEmpty())
    {
        return;
    }
    bool ruleAdded = false;
    int exisitingRuleIndex = -1;
    auto splitted = ruleVale.split(QString::fromUtf8(","));
    for (auto value : splitted)
    {
        value = value.trimmed();
        if(value.isEmpty())
        {
           continue;
        }
        int currentRuleIndex = ruleExist(targetType, wildCard, value);
        if (currentRuleIndex != -1)
        {
           exisitingRuleIndex = currentRuleIndex;
           continue;
        }
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        if (targetType == TargetType::EXTENSION) {
            mMegaIgnoreManager->addExtensionRule(MegaIgnoreNameRule::Class::EXCLUDE, value);
            ruleAdded = true;
            endInsertRows();
            continue;
        }
        mMegaIgnoreManager->addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                        value,
                                        targetType == TargetType::FILE
                                            ? (MegaIgnoreNameRule::Target::f)
                                            : (MegaIgnoreNameRule::Target::d),
                                        static_cast<MegaIgnoreNameRule::WildCardType>(wildCard));
        ruleAdded = true;
        endInsertRows();
    }
    if (ruleAdded)
    {
        emit newRuleAdded(rowCount() - 1);
    }
    else if( exisitingRuleIndex != -1)
    {
        emit newRuleAdded(exisitingRuleIndex);
    }
}

void ExclusionRulesModel::setFolder(const QString &folderName)
{
    beginResetModel();
    if(!mMegaIgnoreManager)
    {
        mMegaIgnoreManager = std::make_shared<MegaIgnoreManager>(folderName, true);
        checkEnableAllStatus();
        endResetModel();
        return;
    }
    mMegaIgnoreManager->setInputDirPath(folderName);
    checkEnableAllStatus();
    endResetModel();
}

void ExclusionRulesModel::applyChanges()
{
    mMegaIgnoreManager->applyChanges(false);
}

Qt::CheckState ExclusionRulesModel::getEnabledRulesStatus() const
{
    return mEnableAllRulesState;
}

int ExclusionRulesModel::ruleExist(int targetType, int wildCard, QString ruleVale)
{
    for(int ruleIndex = 0; ruleIndex < rowCount(); ++ruleIndex)
    {
        if (const auto rule = std::dynamic_pointer_cast<MegaIgnoreNameRule>(mMegaIgnoreManager->getNameRule(ruleIndex)))
        {
            // In extension we do not care about wildcard
            if(targetType == TargetType::EXTENSION)
            {
                if(getTargetType(rule) == targetType && rule->getDisplayText() == ruleVale)
                {
                    return ruleIndex;
                }
            }
            if(getTargetType(rule) == targetType && rule->getWildCardType() == static_cast<MegaIgnoreNameRule::WildCardType>(wildCard) && rule->getDisplayText() == ruleVale)
            {
                return ruleIndex;
            }
        }
    }
    return -1;
}
