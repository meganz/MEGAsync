#ifndef MEGAIGNOREMANAGER_H
#define MEGAIGNOREMANAGER_H

#include <QString>
#include <QFile>
#include <QMap>
#include <QMetaEnum>
#include <memory>

class MegaIgnoreManager;

class MegaIgnoreRule : public QObject
{
    Q_OBJECT

public:
    enum RuleType
    {
        NAMERULE,
        EXTENSIONRULE,
        SIZERULE,
        INVALIDRULE
    };
    explicit MegaIgnoreRule(const QString& rule, bool isCommented)
        : mRule(rule),
        mIsDirty(false),
        mIsCommented(isCommented),
        mIsDeleted(false)
    {}
    virtual ~MegaIgnoreRule() = default;
    virtual bool isValid() const { return true; }
    virtual RuleType ruleType() const = 0;

    bool isEqual(const QString& ruleAsStringToCompare) const;

    void setCommented(bool newIsCommented);
    bool isCommented() const;

    virtual QString getModifiedRule() const { return mRule; }
    virtual QString getDisplayText() const { return mRule; }

    const QString& originalRule() const;

    bool isDeleted() const;
    void setDeleted(bool newIsDeleted);

    void markAsDirty();
    bool isDirty() const;

protected:
    RuleType mRuleType;
    QString mRule;
    bool mIsDirty;
    bool mIsCommented;
    bool mIsDeleted;
};

class MegaIgnoreNameRule : public MegaIgnoreRule
{
    Q_OBJECT

public:
    enum class Class
    {
        EXCLUDE,
        INCLUDE
    };

    enum class Target
    {
        NONE,
        a,
        d,
        f,
        s
    };
    Q_ENUM(Target)

    enum class Type
    {
        NONE,
        N,
        n,
        p
    };
    Q_ENUM(Type)

    enum class Strategy
    {
        NONE,
        g,
        r
    };
    Q_ENUM(Strategy)

    enum class WildCardType
    {
        EQUAL,
        STARTSWITH,
        ENDSWITH,
        CONTAINS
    };
    Q_ENUM(WildCardType)

    explicit MegaIgnoreNameRule(const QString& rule, bool isCommented);
    explicit MegaIgnoreNameRule(const QString& pattern, Class classType, Target target = Target::NONE, Type type = Type::NONE, Strategy strategy = Strategy::NONE);
    QString getModifiedRule() const override;
    QString getDisplayText() const override { return mPattern; }
    RuleType ruleType() const override { return RuleType::NAMERULE;}
    WildCardType wildCardType() { return mWildCardType; }
    Target getTarget() { return mTarget; }

protected:
    QString mPattern;  

private:
    void fillWildCardType(const QString& rightSide);
    template <class EnumType>
    bool detectValue(QString character, EnumType* value, Qt::CaseSensitivity caseSensitive)
    {
        if (static_cast<int>((*value)) > 0)
        {
            return false;
        }

        QMetaEnum e = QMetaEnum::fromType<EnumType>();
        for (int i = 0; i < e.keyCount(); i++)
        {
            QString s(QString::fromUtf8(e.key(i))); // enum name as string
            if (s.compare(character, caseSensitive) == 0)
            {
                (*value) = static_cast<EnumType>(i);
                return true;
            }
        }
        return false;
    }

    Class mClass;
    Target mTarget = Target::NONE;
    Type mType = Type::NONE;
    Strategy mStrategy = Strategy::NONE;
    WildCardType mWildCardType = WildCardType::EQUAL;
};

class MegaIgnoreExtensionRule : public MegaIgnoreNameRule
{
public:
    MegaIgnoreExtensionRule(const QString& rule, bool isCommented);
    explicit MegaIgnoreExtensionRule(Class classType, const QString& extension);

    RuleType ruleType() const override { return RuleType::EXTENSIONRULE; }
    const QString& extension() const;

private:
    QString mExtension;
};

class MegaIgnoreSizeRule : public MegaIgnoreRule
{
    Q_OBJECT


public:
    static const QString LOW_STRING;
    static const QString HIGH_STRING;

    enum Threshold
    {
        LOW,
        HIGH
    };

    enum UnitTypes
    {
        B,
        K,
        M,
        G
    };
    Q_ENUM(UnitTypes)

    explicit MegaIgnoreSizeRule(const QString& rule, bool isCommented);
    explicit MegaIgnoreSizeRule(Threshold type);

    RuleType ruleType() const override { return RuleType::SIZERULE; }
    QString getModifiedRule() const override;

    int value() const;
    UnitTypes unit() const;

    static QStringList getUnitsForDisplay();

    void setValue(uint64_t newValue);
    void setUnit(int newUnit);

private:
    uint64_t mValue = 1;
    UnitTypes mUnit = UnitTypes::B;
    Threshold mThreshold = LOW;
};

class MegaIgnoreInvalidRule : public MegaIgnoreRule
{
    Q_OBJECT

public:
    explicit MegaIgnoreInvalidRule(const QString& rule, bool isCommented)
        :MegaIgnoreRule(rule, isCommented) {}

    bool isValid() const override { return false; }
    RuleType ruleType() const override { return RuleType::INVALIDRULE; }
    virtual QString getModifiedRule() const override{ return isCommented() && (!mRule.startsWith(QLatin1String("#")))? QLatin1String("#") + mRule: mRule; }

};

class MegaIgnoreManager
{
public:
    MegaIgnoreManager(const QString& syncLocalFolder, bool createIfNotExist);

    static bool isValid(const QString& syncLocalFolder);

    std::shared_ptr<MegaIgnoreRule> getRuleByOriginalRule(const QString& originalRule);

    std::shared_ptr<MegaIgnoreSizeRule> getLowLimitRule() const;
    std::shared_ptr<MegaIgnoreSizeRule> getHighLimitRule() const;

    std::shared_ptr<MegaIgnoreNameRule> getIgnoreSymLink() const;

    QList<std::shared_ptr<MegaIgnoreNameRule>> getNameRules() const;

    QList<std::shared_ptr<MegaIgnoreRule>> getAllRules() const;
    std::shared_ptr<MegaIgnoreRule> findRule(const QString& ruleToCompare);
    static MegaIgnoreRule::RuleType getRuleType(const QString& line);
    QStringList getExcludedExtensions() const;

    void parseIgnoresFile();

    void enableExtensions(bool state);
    std::shared_ptr<MegaIgnoreNameRule> addIgnoreSymLinksRule();
    std::shared_ptr<MegaIgnoreNameRule> addIgnoreSymLinkRule(const QString& pattern);
    std::shared_ptr<MegaIgnoreNameRule> addNameRule(MegaIgnoreNameRule::Class classType, const QString& pattern, MegaIgnoreNameRule::Target targetType = MegaIgnoreNameRule::Target::NONE);

    enum ApplyChangesError
    {
        Ok,
        NoUpdateNeeded,
        NoWritePermission
    };

    MegaIgnoreManager::ApplyChangesError applyChanges(bool updateExtensions = false, const QStringList& updatedExtensions = {});

    void setOutputIgnorePath(const QString& outputPath);

    bool hasChanged() const;

private:
    template <class Type>
    static const std::shared_ptr<Type> convert(const std::shared_ptr<MegaIgnoreRule> data)
    {
        return std::dynamic_pointer_cast<Type>(data);
    }

    template <class Type>
    bool addRule(std::shared_ptr<Type> rule)
    {
        const auto ruleText = rule->getModifiedRule();
        auto alreadyExists = findRule(ruleText);
        if (!alreadyExists || ruleText.isEmpty())
        {
            mRules.append(rule);
        }
        //Return if the addition was succesful
        return !alreadyExists;
    }

    QString mMegaIgnoreFile;
    QString mOutputMegaIgnoreFile;
    QList<std::shared_ptr<MegaIgnoreRule>> mRules;
    QMap<QString, std::shared_ptr<MegaIgnoreRule> > mExtensionRules;

    std::shared_ptr<MegaIgnoreSizeRule> mLowLimitRule;
    std::shared_ptr<MegaIgnoreSizeRule> mHighLimitRule;

    std::shared_ptr<MegaIgnoreNameRule> mIgnoreSymLinkRule;
    
    QString mIgnoreCRC;

};

#endif // MEGAIGNOREMANAGER_H
