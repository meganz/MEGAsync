#include "MegaIgnoreRules.h"

#include "Platform.h"
#include "Utilities.h"

#include <QRegularExpression>

const char MegaIgnoreRule::SIZE_RULE_LEFT_SIDE_REG_EX[] = "^#*(exclude-(larger|smaller))$";
const char MegaIgnoreRule::SIZE_RULE_RIGHT_SIDE_REG_EX[] = "^[0-9]+[kmg]?$";
const char MegaIgnoreRule::LARGE_SIZE_LEFT_SIDE_REG_EX[] = "#*(exclude-larger):*";
const char MegaIgnoreRule::SMALL_SIZE_LEFT_SIDE_REG_EX[] = "#*(exclude-smaller):*";
const char MegaIgnoreRule::IGNORE_SYM_LINK[] = "-s:*";
const char MegaIgnoreRule::NON_SIZE_LEFT_SIDE_REG_EX[] = "^#*[+-][]adfsNnpGgRr]*$";
const char MegaIgnoreRule::RIGHT_SIDE_TYPE_RULE_REG_EX[] = "^\\*\\..*";
const char MegaIgnoreRule::CAPTURE_EXTENSION_REG_EX[] = "\\.(.*)";

const auto asterisk = QString::fromUtf8("*");
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
                    if (detectValue(chr, &mTarget))
                    {
                        continue;
                    }

                    if (detectValue(chr, &mType))
                    {
                        continue;
                    }

                    if (detectValue(chr, &mStrategy))
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

MegaIgnoreNameRule::MegaIgnoreNameRule(const QString& pattern,
                                       Class classType,
                                       Target target,
                                       Type type,
                                       Strategy strategy,
                                       WildCardType wildcard):
    MegaIgnoreRule(QString(), false),
    mPattern(pattern),
    mClass(classType),
    mTarget(target),
    mType(type),
    mStrategy(strategy),
    mWildCardType(wildcard)
{
    markAsDirty();
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
        // Extension rules we ignore the wild card type
        if(mRuleType != EXTENSIONRULE)
        {
            switch (mWildCardType) {
            case MegaIgnoreNameRule::WildCardType::STARTSWITH:
                rule.append(mPattern + asterisk);
                break;
            case MegaIgnoreNameRule::WildCardType::ENDSWITH:
                rule.append(asterisk + mPattern);
                break;
            case MegaIgnoreNameRule::WildCardType::CONTAINS:
                rule.append(asterisk + mPattern + asterisk);
                break;
            default:
                rule.append(mPattern);
                break;
            }
        }
        else
        {
            rule.append(asterisk + mPattern);
        }
        return rule;
    }
    else
    {
        return MegaIgnoreRule::getModifiedRule();
    }
}

void MegaIgnoreNameRule::setTarget(Target target)
{
    if(target == mTarget)
    {
        return;
    }
    mTarget = target;
    markAsDirty();
}

MegaIgnoreNameRule::WildCardType MegaIgnoreNameRule::getWildCardType()
{
    return mWildCardType;
}

void MegaIgnoreNameRule::setWildCardType(WildCardType wildCard)
{
    if(wildCard == mWildCardType)
    {
        return;
    }
    mWildCardType = wildCard;
    markAsDirty();
}

void MegaIgnoreNameRule::setPattern(const QString &pattern)
{
    if(pattern == mPattern)
    {
        return;
    }
    mPattern = pattern;
    markAsDirty();
}

void MegaIgnoreNameRule::fillWildCardType(const QString& rightSidePart)
{
    static const auto regexElements = QRegularExpression(QLatin1String("[?\\[\\]{}+()^$|\\\\]"));
    const bool hasRegexElements = rightSidePart.contains(regexElements);
    if (hasRegexElements)
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::WILDCARD;
        return;
    }
    const int asteriskCount = rightSidePart.count(QLatin1String("*"));
    if (asteriskCount == 0)
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::EQUAL;
    }
    else if (asteriskCount == 1)
    {
        if (rightSidePart.startsWith(QLatin1String("*")))
        {
            mWildCardType = MegaIgnoreNameRule::WildCardType::ENDSWITH;
        }
        else if (rightSidePart.endsWith(QLatin1String("*")))
        {
            mWildCardType = MegaIgnoreNameRule::WildCardType::STARTSWITH;
        }
        else
        {
            mWildCardType = MegaIgnoreNameRule::WildCardType::WILDCARD;
        }
    }
    else if ((asteriskCount == 2) && rightSidePart.startsWith(QLatin1String("*")) &&
             rightSidePart.endsWith(QLatin1String("*")))
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::CONTAINS;
    }
    else
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::WILDCARD;
    }
    if (mWildCardType != MegaIgnoreNameRule::WildCardType::WILDCARD)
    {
        mPattern = mPattern.remove(asterisk);
    }
}

MegaIgnoreNameRule::Strategy MegaIgnoreNameRule::getStrategy() const
{
    return mStrategy;
}

void MegaIgnoreNameRule::setStrategy(Strategy newStrategy)
{
    mStrategy = newStrategy;
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

void MegaIgnoreExtensionRule::setPattern(const QString &pattern)
{
    if(mPattern == pattern)
    {
        return;
    }
    if(pattern.startsWith(QLatin1String(".")))
    {
        mPattern = pattern;
        mExtension = pattern;
        mExtension.remove(0,1);
    }
    else
    {
        mExtension = pattern;
        mPattern = QLatin1String(".") + pattern;
    }
    markAsDirty();
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
            try
            {
                mValue = value.toULongLong();
            }
            catch (...)
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error loading .megaignore").toUtf8());
            }

            auto unit = size.remove(value).toUpper().trimmed();
            if (!unit.isEmpty())
            {
                EnumConversions<UnitTypes> convertEnum;
                mUnit = convertEnum.getEnum(unit);
            }
            computeMaximumUnit();
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
    } else {
        return MegaIgnoreRule::getModifiedRule();
    }
}

unsigned long long MegaIgnoreSizeRule::value() const
{
    return mValue;
}

double MegaIgnoreSizeRule::valueInBytes()
{
    auto baseUnitSize = Platform::getInstance()->getBaseUnitsSize();
    auto doubleValue(static_cast<double>(mValue));

    switch (mUnit) {
        case MegaIgnoreSizeRule::UnitTypes::G:
            return doubleValue*std::pow(baseUnitSize, 3);
        case MegaIgnoreSizeRule::UnitTypes::M:
            return doubleValue*std::pow(baseUnitSize, 2);
        case MegaIgnoreSizeRule::UnitTypes::K:
            return doubleValue*std::pow(baseUnitSize, 1);
        default:
            return doubleValue;
    }
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

void MegaIgnoreSizeRule::setValue(unsigned long long newValue)
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

void MegaIgnoreSizeRule::computeMaximumUnit()
{
    for(int unitIterator = mUnit + 1; unitIterator <= UnitTypes::G; ++unitIterator)
    {
        if((long long)mValue % (1024) ==0)
        {
            mValue /= 1024;
            mUnit = static_cast<UnitTypes>(unitIterator);
        }
        else
        {
            break;
        }
    }
}
