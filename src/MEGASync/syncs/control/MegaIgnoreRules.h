#ifndef MEGAIGNORERULES_H
#define MEGAIGNORERULES_H

#include <QString>
#include <QMetaEnum>

class MegaIgnoreManager;

class MegaIgnoreRule : public QObject
{
    Q_OBJECT

public:
    static const char SIZE_RULE_LEFT_SIDE_REG_EX[];
    static const char SIZE_RULE_RIGHT_SIDE_REG_EX[];
    static const char LARGE_SIZE_LEFT_SIDE_REG_EX[];
    static const char SMALL_SIZE_LEFT_SIDE_REG_EX[];
    static const char IGNORE_SYM_LINK[];
    static const char NON_SIZE_LEFT_SIDE_REG_EX[] ;
    static const char RIGHT_SIDE_TYPE_RULE_REG_EX[];
    static const char CAPTURE_EXTENSION_REG_EX[];

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

    static const QStringList &getUnitsForDisplay();

    void setValue(int newValue);
    void setUnit(int newUnit);

private:
    int mValue = 1;
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

#endif // MEGAIGNORERULES_H
