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

    MegaIgnoreRule(MegaIgnoreManager* manager, const QString& rule, bool isCommented, int row)
        : mRule(rule),
          mRow(row),
          mIsDirty(false),
          mIsCommented(isCommented),
          mManager(manager)
    {}

    virtual QString getRuleAsString() { return mRule;}
    RuleType ruleType(){return mRuleType;}

    void setCommented(bool newIsCommented);
    bool isCommented() const;

protected:
    RuleType mRuleType;
    QString mRule;
    int mRow;
    bool mIsDirty;
    bool mIsCommented;
    MegaIgnoreManager* mManager;
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

    MegaIgnoreNameRule(MegaIgnoreManager* manager, const QString& rule, bool isCommented, int row);
    QString getRuleAsString() override;

protected:
    QString mRightSideRule;

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
    MegaIgnoreExtensionRule(MegaIgnoreManager* manager, const QString& rule, bool isCommented, int row);

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

    MegaIgnoreSizeRule(MegaIgnoreManager* manager, const QString& rule, bool isCommented, int row);
    MegaIgnoreSizeRule(MegaIgnoreManager* manage, Threshold type);
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

    std::shared_ptr<MegaIgnoreSizeRule> getLowLimitRule();
    std::shared_ptr<MegaIgnoreSizeRule> getHighLimitRule();

private:
    friend class MegaIgnoreSizeRule;
    friend class MegaIgnoreRule;
    int updateRow(int row, const QString& rule);

    QString mMegaIgnoreFile;
    QList<std::shared_ptr<MegaIgnoreRule>> mRules;

    std::shared_ptr<MegaIgnoreSizeRule> mLowLimitRule;
    std::shared_ptr<MegaIgnoreSizeRule> mHighLimitRule;

    int mLastRow;
};

#endif // MEGAIGNOREMANAGER_H
