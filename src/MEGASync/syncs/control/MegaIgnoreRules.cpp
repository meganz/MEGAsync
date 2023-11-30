#include "MegaIgnoreRules.h"

#include <Utilities.h>
#include "MegaApplication.h"

#include <QRegularExpression>

const char MegaIgnoreRule::SIZE_RULE_LEFT_SIDE_REG_EX[] = "^#*(exclude-(larger|smaller))$";
const char MegaIgnoreRule::SIZE_RULE_RIGHT_SIDE_REG_EX[] = "^[0-9]+[kmg]?$";
const char MegaIgnoreRule::LARGE_SIZE_LEFT_SIDE_REG_EX[] = "#*(exclude-larger):*";
const char MegaIgnoreRule::SMALL_SIZE_LEFT_SIDE_REG_EX[] = "#*(exclude-smaller):*";
const char MegaIgnoreRule::IGNORE_SYM_LINK[] = "-s:*";
const char MegaIgnoreRule::NON_SIZE_LEFT_SIDE_REG_EX[] = "^#*[+-][]adfsNnpGgRr]*$";
const char MegaIgnoreRule::RIGHT_SIDE_TYPE_RULE_REG_EX[] = "^\\*\\..*";
const char MegaIgnoreRule::CAPTURE_EXTENSION_REG_EX[] = "\\.(.*)";

////////////////MEGA IGNORE RULE
bool MegaIgnoreRule::isEqual(const QString& ruleAsStringToCompare) const
{
    QString ruleAsString(getModifiedRule());
    return ruleAsString.compare(ruleAsStringToCompare) == 0;
}

void MegaIgnoreRule::setCommented(bool newIsCommented)
{
    if (newIsCommented != mIsCommented)
    {
        mIsDirty = true;
        mIsCommented = newIsCommented;
    }
}

bool MegaIgnoreRule::isCommented() const
{
    return mIsCommented;
}

const QString& MegaIgnoreRule::originalRule() const
{
    return mRule;
}

bool MegaIgnoreRule::isDeleted() const
{
    return mIsDeleted;
}

void MegaIgnoreRule::setDeleted(bool newIsDeleted)
{
    mIsDeleted = newIsDeleted;
}

void MegaIgnoreRule::markAsDirty()
{
    mIsDirty = true;
}

bool MegaIgnoreRule::isDirty() const
{
    return mIsDirty;
}

/////////////////MEGA IGNORE NAME RULE
MegaIgnoreNameRule::MegaIgnoreNameRule(const QString& rule, bool isCommented)
    :MegaIgnoreRule(rule, isCommented)
{
    mRuleType = RuleType::NAMERULE;

    auto ruleSplitted = rule.split(QLatin1String(":"));
    if (ruleSplitted.size() > 1)
    {
        //First part -> class, target, type, strategy
        QString leftSide(ruleSplitted.at(0));
        leftSide.remove(QRegExp(QLatin1String("^#+")));
        if (!leftSide.isEmpty())
        {
            auto charCounter(0);
            for (auto chr : leftSide)
            {
                if (charCounter == 0)
                {
                    mClass = chr == QLatin1String("-") ? Class::EXCLUDE : Class::INCLUDE;
                }
                else
                {
                    if (detectValue(chr, &mTarget, Qt::CaseSensitive))
                    {
                        continue;
                    }

                    if (detectValue(chr, &mType, Qt::CaseSensitive))
                    {
                        continue;
                    }

                    if (detectValue(chr, &mStrategy, Qt::CaseInsensitive))
                    {
                        continue;
                    }
                }

                charCounter++;
            }
        }
        ruleSplitted.pop_front();
        mPattern = ruleSplitted.join(QLatin1String(":"));
        fillWildCardType(mPattern);
    }
}

MegaIgnoreNameRule::MegaIgnoreNameRule(const QString& pattern, Class classType, Target target, Type type, Strategy strategy) :
    mPattern(pattern),
    mClass(classType),
    mTarget(target),
    mType(type),
    mStrategy(strategy),
    MegaIgnoreRule(QString(), false)
{
    markAsDirty();
    fillWildCardType(pattern);
}

QString MegaIgnoreNameRule::getModifiedRule() const
{
    if (mIsDirty)
    {
        QString rule;
        if (isCommented())
        {
            rule.append(QLatin1String("#"));
        }

        rule.append(mClass == Class::EXCLUDE ? QLatin1String("-") : QLatin1String("+"));
        if (mTarget != Target::NONE)
        {
            EnumConversions<Target> convertEnum;
            rule.append(convertEnum.getString(mTarget));
        }
        if (mType != Type::NONE)
        {
            EnumConversions<Type> convertEnum;
            rule.append(convertEnum.getString(mType));
        }
        if (mStrategy != Strategy::NONE)
        {
            EnumConversions<Strategy> convertEnum;
            rule.append(convertEnum.getString(mStrategy));
        }
        rule.append(QLatin1String(":"));
        rule.append(mPattern);

        return rule;
    }
    else
    {
        return MegaIgnoreRule::getModifiedRule();
    }
}

void MegaIgnoreNameRule::fillWildCardType(const QString& rightSidePart)
{
    const int asteriskCount = rightSidePart.count(QLatin1String("*"));
    if (asteriskCount == 0)
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::EQUAL;
    }
    else if (asteriskCount > 1)
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::CONTAINS;
    }
    else if (rightSidePart.startsWith(QLatin1String("*")))
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::ENDSWITH;
    }
    else
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::STARTSWITH;
    }
}

////////////////MEGA IGNORE EXTENSION RULE
MegaIgnoreExtensionRule::MegaIgnoreExtensionRule(const QString& rule, bool isCommented)
    : MegaIgnoreNameRule(rule, isCommented)
{
    mRuleType = RuleType::EXTENSIONRULE;
    auto extensionSplitter(mPattern.split(QLatin1String(".")));
    QRegExp captureExtension{ QLatin1String(MegaIgnoreRule::CAPTURE_EXTENSION_REG_EX) };
    if (captureExtension.indexIn(rule) != -1)
    {
        mExtension = captureExtension.cap(1); // Capture extension
    }
}

MegaIgnoreExtensionRule::MegaIgnoreExtensionRule(Class classType, const QString& extension)
                                                :MegaIgnoreNameRule(QString::fromUtf8("*.") + extension, classType)
{
    mRuleType = RuleType::EXTENSIONRULE;
    mExtension = extension;
}

const QString& MegaIgnoreExtensionRule::extension() const
{
    return mExtension;
}

////////////////MEGA IGNORE SIZE RULE
static const QLatin1String LOW_STRING = QLatin1String("exclude-smaller");
static const QLatin1String HIGH_STRING = QLatin1String("exclude-larger");

MegaIgnoreSizeRule::MegaIgnoreSizeRule(const QString& rule, bool isCommented)
    :MegaIgnoreRule(rule, isCommented)
{
    mRuleType = RuleType::SIZERULE;

    auto ruleSplitted = rule.split(QLatin1String(":"));
    if (ruleSplitted.size() == 2)
    {
        if(isCommented && rule.startsWith(QLatin1String("#")))
        {
            auto leftRule = ruleSplitted.at(0);
            leftRule.remove(0,1);
            ruleSplitted[0] = leftRule;
        }

        auto type(ruleSplitted.at(0));
        if (type == HIGH_STRING)
        {
            mThreshold = Threshold::HIGH;
        }
        else if (type == LOW_STRING)
        {
            mThreshold = Threshold::LOW;
        }

        auto size(ruleSplitted.at(1));

        QRegularExpression rx(QLatin1String("[0-9]+"));
        QRegularExpressionMatch match = rx.match(size);
        if (match.hasMatch())
        {
            auto value = match.captured(0);
            mValue = value.toUInt();
            auto unit = size.remove(value).toUpper().trimmed();
            if (!unit.isEmpty())
            {
                EnumConversions<UnitTypes> convertEnum;
                mUnit = convertEnum.getEnum(unit);
            }
        }
    }
}

MegaIgnoreSizeRule::MegaIgnoreSizeRule(Threshold type)
    :MegaIgnoreRule(QString(), true)
    , mThreshold(type)
{
    mRuleType = RuleType::SIZERULE;
}

QString MegaIgnoreSizeRule::getModifiedRule() const
{
    if (mIsDirty)
    {
        QString rule;
        if (isCommented())
        {
            rule.append(QLatin1String("#"));
        }
        rule.append(mThreshold == Threshold::LOW ? LOW_STRING : HIGH_STRING);
        rule.append(QLatin1String(":"));
        rule.append(QString::number(mValue));
        EnumConversions<UnitTypes> convertEnum;
        rule.append(mUnit == UnitTypes::B ? QString() : convertEnum.getString(mUnit));
        return rule;
    }
    else
    {
        return MegaIgnoreRule::getModifiedRule();
    }
}

int MegaIgnoreSizeRule::value() const
{
    return mValue;
}

MegaIgnoreSizeRule::UnitTypes MegaIgnoreSizeRule::unit() const
{
    return mUnit;
}

const QStringList& MegaIgnoreSizeRule::getUnitsForDisplay()
{
    static const auto units =  QStringList() << QApplication::translate("Sizes", "B")
                                            << QApplication::translate("Sizes", "KB")
                                            << QApplication::translate("Sizes", "MB")
                                            << QApplication::translate("Sizes", "GB");
    return units;
}

void MegaIgnoreSizeRule::setValue(int newValue)
{
    if (mValue != newValue)
    {
        mIsDirty = true;
        mValue = newValue;
    }
}

void MegaIgnoreSizeRule::setUnit(int newUnit)
{
    if (newUnit != mUnit)
    {
        mIsDirty = true;
        mUnit = static_cast<UnitTypes>(newUnit);
    }
}
