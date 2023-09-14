#ifndef MEGAIGNOREMANAGER_H
#define MEGAIGNOREMANAGER_H

#include <QString>
#include <QFile>

#include <memory>

class MegaIgnoreManager;

class MegaIgnoreRule : public QObject
{
    Q_OBJECT

public:
    enum RuleType
    {
        NameRule,
        ExtensionRule,
        SizeRule
    };

    MegaIgnoreRule(const QString& rule, bool isCommented)
        : mRule(rule),
          mIsDirty(false),
          mIsCommented(isCommented)
    {}

    virtual bool isValid(){return true;}
    virtual QString getRuleAsString() { return mRule;}
    virtual RuleType ruleType() = 0;

    void setCommented(bool newIsCommented);
    bool isCommented() const;

    void setId(int newId);
    int getId() const;

protected:
    RuleType mRuleType;
    QString mRule;
    bool mIsDirty;
    bool mIsCommented;
    int id;
};

class MegaIgnoreNameRule : public MegaIgnoreRule
{
    Q_OBJECT

public:
    enum Class
    {
        Exclude,
        Include
    };

    enum class Target
    {
        None,
        a,
        d,
        f,
        s
    };
    Q_ENUM(Target)

    enum class Type
    {
        None,
        N,
        n,
        p
    };
    Q_ENUM(Type)

    enum class Strategy
    {
        None,
        g,
        r
    };
    Q_ENUM(Strategy)

    MegaIgnoreNameRule(const QString& rule, bool isCommented);
    QString getRuleAsString() override;

protected:
    QString mPattern;

private:
    template <class EnumType>
    bool detectValue(QString character, EnumType* value, Qt::CaseSensitivity caseSensitive)
    {
        if(value > 0)
        {
            return false;
        }

        QMetaEnum e = QMetaEnum::fromType<EnumType>();
        for (int i = 0; i < e.keyCount(); i++)
        {
            QString s(QString::fromUtf8(e.key(i))); // enum name as string
            if(s.compare(character, caseSensitive))
            {
                (*value) = static_cast<EnumType>(i);
                return true;
            }
        }

        return false;
    }

    Class mClass;
    Target mTarget = Target::None;
    Type mType = Type::None;
    Strategy mStrategy = Strategy::None;
};

class MegaIgnoreExtensionRule : public MegaIgnoreNameRule
{
public:
    MegaIgnoreExtensionRule(const QString& rule, bool isCommented);

    const QString &extension() const;

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
        Low,
        High
    };

    enum UnitTypes
    {
        B,
        K,
        M,
        G
    };
    Q_ENUM(UnitTypes)

    MegaIgnoreSizeRule(const QString& rule, bool isCommented);
    MegaIgnoreSizeRule(Threshold type);

    bool isValid() override;
    QString getRuleAsString() override;

    int value() const;
    UnitTypes unit() const;

    static QStringList getUnitsForDisplay();

    void setValue(uint64_t newValue);
    void setUnit(int newUnit);

private:
    uint64_t mValue = 0;
    UnitTypes mUnit = UnitTypes::B;
    Threshold mThreshold = Low;
};

class MegaIgnoreManager
{
public:
    MegaIgnoreManager(const QString &syncLocalFolder);

    std::shared_ptr<MegaIgnoreSizeRule> getLowLimitRule() const;
    std::shared_ptr<MegaIgnoreSizeRule> getHighLimitRule() const;

    QList<std::shared_ptr<MegaIgnoreNameRule>> getNameRules() const;

    QStringList getExcludedExtensions() const;
    void enableExtensions(bool state);

    void applyChanges();

private:
    template <class Type>
    static const std::shared_ptr<Type> convert(const std::shared_ptr<MegaIgnoreRule> data)
    {
        return std::dynamic_pointer_cast<Type>(data);
    }

    QString mMegaIgnoreFile;
    QList<std::shared_ptr<MegaIgnoreRule>> mRules;

    std::shared_ptr<MegaIgnoreSizeRule> mLowLimitRule;
    std::shared_ptr<MegaIgnoreSizeRule> mHighLimitRule;
};

#endif // MEGAIGNOREMANAGER_H
