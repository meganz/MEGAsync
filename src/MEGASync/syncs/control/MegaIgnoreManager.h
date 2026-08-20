#ifndef MEGAIGNOREMANAGER_H
#define MEGAIGNOREMANAGER_H

#include "MegaIgnoreRules.h"

#include <QFile>
#include <QMap>
#include <QMetaEnum>
#include <QString>

#include <functional>
#include <memory>

class MegaIgnoreManager
{
public:
    static constexpr char MEGA_IGNORE_FILE_NAME[] = ".megaignore";
    static constexpr char MEGA_IGNORE_DEFAULT_FILE_NAME[] = ".megaignore.default";

    explicit MegaIgnoreManager(const QString& syncLocalFolder, bool createIfNotExist);
    explicit MegaIgnoreManager() = default;

    static bool isValid(const QString& syncLocalFolder);

    std::shared_ptr<MegaIgnoreSizeRule> getLowLimitRule() const;

    std::shared_ptr<MegaIgnoreSizeRule> getHighLimitRule() const;

    std::shared_ptr<MegaIgnoreNameRule> getIgnoreSymLink() const;

    QList<std::shared_ptr<MegaIgnoreRule>> getAllRules() const;

    std::shared_ptr<MegaIgnoreRule> getNameRule(int index) const;

    void enableAllNameRules(bool enable);

    int enabledRulesCount();

    void removeRule(std::shared_ptr<MegaIgnoreRule> rule);

    std::shared_ptr<MegaIgnoreRule> findRule(const QString& ruleToCompare);
    static MegaIgnoreRule::RuleType getRuleType(const QString& line);
    QStringList getExcludedExtensions() const;

    void parseIgnoresFile();

    std::shared_ptr<MegaIgnoreNameRule> addIgnoreSymLinksRule();
    std::shared_ptr<MegaIgnoreNameRule> addIgnoreSymLinkRule(const QString& pattern);
    std::shared_ptr<MegaIgnoreNameRule> addNameRule(
        MegaIgnoreNameRule::Class classType,
        QString pattern,
        MegaIgnoreNameRule::Target targetType = MegaIgnoreNameRule::Target::NONE,
        MegaIgnoreNameRule::Type type = MegaIgnoreNameRule::Type::NONE,
        MegaIgnoreNameRule::WildCardType wildCard = MegaIgnoreNameRule::WildCardType::EQUAL);
    void updateNameRuleStrategyAcordingToCaseSensitive(std::shared_ptr<MegaIgnoreNameRule> rule);
    std::shared_ptr<MegaIgnoreExtensionRule> addExtensionRule(MegaIgnoreNameRule::Class classType, const QString& pattern);

    enum class RestoreDefaultsResult
    {
        // Copied .megaignore.default over .megaignore and reapplied Hidden.
        Restored,
        // Copied .megaignore.default over .megaignore, but reapplying Hidden failed. The
        // exclusions themselves were restored correctly; only the file's visibility wasn't.
        RestoredButNotHidden,
        // There's no .megaignore.default to copy from, so .megaignore was removed instead.
        // The SDK only regenerates it on a sync's transition into RUNSTATE_RUNNING, so the
        // caller needs to force that (e.g. suspend then resume the sync) for it to take
        // effect while the sync keeps running.
        RemovedPendingSdkRegeneration,
        Failed,
    };

    RestoreDefaultsResult restoreDefaults();

    // Lets a caller check, before calling restoreDefaults(), whether it is about to hit
    // the RemovedPendingSdkRegeneration case - so it can pause the sync first and avoid a
    // window where the folder is briefly unfiltered while a sync is running.
    bool hasDefaultFile() const;

    bool isDefault() const;

    enum ApplyChangesError
    {
        OK,
        NO_UPDATE_NEEDED,
        NO_WRITE_PERMISSION
    };

    MegaIgnoreManager::ApplyChangesError applyChanges(bool updateExtensions = false, const QStringList& updatedExtensions = {});

    void setOutputIgnorePath(const QString& outputPath);

    void setDefaultIgnorePath(const QString& defaultPath);

    // Overrides how restoreDefaults() reapplies the Hidden attribute; defaults to
    // Platform::getInstance()->setHidden(). Exists so tests can force that step to fail
    // deterministically, the same way setDefaultIgnorePath()/setOutputIgnorePath() exist
    // so tests can point at a temp-dir fixture instead of a real profile directory.
    // Passing an empty function (including nullptr) resets it back to that default.
    void setHiddenApplier(std::function<bool(const QString&)> applier);

    void setInputDirPath(const QString& inputDir, bool createIfNotExist = true);

    bool hasChanged() const;

    int getNameRulesCount() const;

    template <class Type>
    bool addRule(std::shared_ptr<Type> rule)
    {
        const auto ruleText = rule->getModifiedRule();
        auto alreadyExists = findRule(ruleText);
        if (!alreadyExists || ruleText.isEmpty())
        {
            mRules.append(rule);
        }
        const auto type = rule->ruleType();
        if (type == MegaIgnoreRule::EXTENSIONRULE || type == MegaIgnoreRule::NAMERULE)
        {
            mNameRules.append(rule);
        }
        // Return if the addition was succesful
        return !alreadyExists;
    }

private:
    QString getDefaultFilePath() const;
    static QStringList readTrimmedLines(const QString& filePath);
    static bool applyRealHiddenAttribute(const QString& path);

    template<class Type>
    static const std::shared_ptr<Type> convert(const std::shared_ptr<MegaIgnoreRule> data)
    {
        return std::dynamic_pointer_cast<Type>(data);
    }

    QString mMegaIgnoreFile;
    QString mOutputMegaIgnoreFile;
    QString mDefaultMegaIgnoreFile;
    QList<std::shared_ptr<MegaIgnoreRule>> mRules;
    QList<std::shared_ptr<MegaIgnoreRule>> mNameRules;
    QMap<QString, std::shared_ptr<MegaIgnoreRule> > mExtensionRules;

    std::shared_ptr<MegaIgnoreSizeRule> mLowLimitRule;
    std::shared_ptr<MegaIgnoreSizeRule> mHighLimitRule;

    std::shared_ptr<MegaIgnoreNameRule> mIgnoreSymLinkRule;
    
    QString mIgnoreCRC;

    Qt::CaseSensitivity mIsCaseSensitive;

    std::function<bool(const QString&)> mHiddenApplier =
        &MegaIgnoreManager::applyRealHiddenAttribute;
};

#endif // MEGAIGNOREMANAGER_H
