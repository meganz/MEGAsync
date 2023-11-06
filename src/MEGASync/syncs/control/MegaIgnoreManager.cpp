#include "MegaIgnoreManager.h"

#include <Utilities.h>
#include "Preferences.h"
#include "MegaApplication.h"

#include <QDir>
#include <QChar>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QApplication>
#include <QTemporaryFile>
#include <QFileInfo>

namespace
{
    constexpr char SIZE_RULE_LEFT_SIDE_REG_EX[] = "^#*(exclude-(larger|smaller))$";
    constexpr char SIZE_RULE_RIGHT_SIDE_REG_EX[] = "^[0-9]+[kmg]?$";
    constexpr char LARGE_SIZE_LEFT_SIDE_REG_EX[] = "#*(exclude-larger):*";
    constexpr char SMALL_SIZE_LEFT_SIDE_REG_EX[] = "#*(exclude-smaller):*";
    constexpr char IGNORE_SYM_LINK[] = "-s:*";
    constexpr char NON_SIZE_LEFT_SIDE_REG_EX[] = "^#*[+-][]adfsNnpGgRr]*$";
    constexpr char RIGHT_SIDE_TYPE_RULE_REG_EX[] = "^\\*\\..*";
    constexpr char CAPTURE_EXTENSION_REG_EX[] = "\\.(.*)";
}

MegaIgnoreManager::MegaIgnoreManager(const QString& syncLocalFolder, bool createIfNotExist)
{
    const auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    mOutputMegaIgnoreFile = ignorePath;
    mMegaIgnoreFile = ignorePath;
    if (createIfNotExist && !QFile::exists(ignorePath))
    {
        mMegaIgnoreFile = Preferences::instance()->getDataPath() + QDir::separator() + QString::fromUtf8(".megaignore.default");
    }
    parseIgnoresFile();
}

bool MegaIgnoreManager::isValid(const QString& syncLocalFolder)
{
    const auto ignorePath(syncLocalFolder + QDir::separator() + QString::fromUtf8(".megaignore"));
    QFileInfo ignoreInfo(ignorePath);
    if (ignoreInfo.exists())
    {
        QFile ignoreFile(ignorePath);
        auto canBeReadWrite(ignoreFile.open(QIODevice::ReadWrite));
        if (canBeReadWrite)
        {
            ignoreFile.close();
        }
        return canBeReadWrite;
    }
    else
    {
        QDir megaIgnorePath(ignorePath);
        QTemporaryFile* test = new QTemporaryFile(megaIgnorePath.absolutePath() + QDir::separator());
        return test->open();
    }

    return true;
}

void MegaIgnoreManager::parseIgnoresFile()
{
    mRules.clear();
    mLowLimitRule.reset();
    mHighLimitRule.reset();
    mIgnoreSymLinkRule.reset();
    mExtensionRules.clear();

    QFile ignore(mMegaIgnoreFile);
    if (ignore.exists())
    {
        if (ignore.open(QIODevice::ReadOnly))
        {
            QTextStream in(&ignore);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                if (line.trimmed().isEmpty())
                {
                    continue;
                }
                const bool isCommented(line.startsWith(QLatin1String("#")));
                const auto ruleType = getRuleType(line);
                switch (ruleType)
                {
                case MegaIgnoreRule::RuleType::SizeRule:
                {
                    const static QRegularExpression largeSizeRegEx{ QLatin1String(::LARGE_SIZE_LEFT_SIDE_REG_EX) };
                    const static QRegularExpression smallSizeRegEx{ QLatin1String(::SMALL_SIZE_LEFT_SIDE_REG_EX) };
                    if (largeSizeRegEx.match(line).hasMatch())
                    {
                        auto highLimitRule = std::make_shared<MegaIgnoreSizeRule>(line, isCommented);
                        if (!mHighLimitRule || !highLimitRule->isCommented())
                        {
                            mHighLimitRule = highLimitRule;
                        }
                        addRule(highLimitRule);
                    }
                    else if (smallSizeRegEx.match(line).hasMatch())
                    {
                        auto lowLimitRule = std::make_shared<MegaIgnoreSizeRule>(line, isCommented);
                        if (!mLowLimitRule || !lowLimitRule->isCommented())
                        {
                            mLowLimitRule = lowLimitRule;
                        }                      
                        addRule(lowLimitRule);
                    }
                    break;
                }
                case MegaIgnoreRule::RuleType::ExtensionRule:
                {
                    auto extensionRule(std::make_shared<MegaIgnoreExtensionRule>(line, isCommented));
                    addRule(extensionRule);
                    mExtensionRules.insert(extensionRule->extension(), extensionRule);
                    break;
                }
                case MegaIgnoreRule::RuleType::NameRule:
                {
                    std::shared_ptr<MegaIgnoreNameRule> rule = std::make_shared<MegaIgnoreNameRule>(line, isCommented);

                    if (line.compare(QLatin1String(::IGNORE_SYM_LINK)) == 0)
                    {
                        mIgnoreSymLinkRule = rule;
                    }

                    addRule(rule);

                    break;
                }
                default:
                    addRule(std::make_shared<MegaIgnoreInvalidRule>(line, isCommented));
                    break;
                }
            }
            ignore.close();
        }

        if (!mLowLimitRule)
        {
            mLowLimitRule = std::make_shared<MegaIgnoreSizeRule>(MegaIgnoreSizeRule::Threshold::Low);
            addRule(mLowLimitRule);
        }
        if (!mHighLimitRule)
        {
            mHighLimitRule = std::make_shared<MegaIgnoreSizeRule>(MegaIgnoreSizeRule::Threshold::High);
            addRule(mHighLimitRule);
        }
    }

    std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRC(mMegaIgnoreFile.toStdString().c_str()));
    mIgnoreCRC = QString::fromUtf8(crc.get());
}

std::shared_ptr<MegaIgnoreRule> MegaIgnoreManager::getRuleByOriginalRule(const QString& originalRule)
{
    foreach(auto & rule, mRules)
    {
        if (rule->originalRule() == originalRule)
        {
            return rule;
        }
    }

    return nullptr;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getLowLimitRule() const
{
    return mLowLimitRule;
}

std::shared_ptr<MegaIgnoreSizeRule> MegaIgnoreManager::getHighLimitRule() const
{
    return mHighLimitRule;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::getIgnoreSymLink() const
{
    return mIgnoreSymLinkRule;
}

QList<std::shared_ptr<MegaIgnoreRule>> MegaIgnoreManager::getAllRules() const
{
    return mRules;
}

std::shared_ptr<MegaIgnoreRule> MegaIgnoreManager::findRule(const QString& ruleToCompare)
{
    auto allRules(getAllRules());
    foreach(auto rule, allRules)
    {
        if (rule->isEqual(ruleToCompare))
        {
            return rule;
        }
    }

    return nullptr;
}

MegaIgnoreRule::RuleType MegaIgnoreManager::getRuleType(const QString& line)
{
    const auto lineSplitted(line.split(QLatin1String(":")));
    if (lineSplitted.size() < 2)
    {
        return MegaIgnoreRule::RuleType::InvalidRule;
    }
    const QString leftSide = lineSplitted[0];
    const QString rightSide = lineSplitted.mid(1).join(QLatin1String(":"));

    // Check size rule 
    static const QRegularExpression sizeRuleLeftSide(QLatin1String(::SIZE_RULE_LEFT_SIDE_REG_EX), QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression sizeRuleRightSide(QLatin1String(::SIZE_RULE_RIGHT_SIDE_REG_EX), QRegularExpression::CaseInsensitiveOption);
    if (sizeRuleLeftSide.match(leftSide).hasMatch() && sizeRuleRightSide.match(rightSide).hasMatch())
    {
        return MegaIgnoreRule::RuleType::SizeRule;
    }
    // Check if valid left side 
    const static  QRegularExpression nonSizeRuleRegularExpression{ QLatin1String(::NON_SIZE_LEFT_SIDE_REG_EX) };
    if (!nonSizeRuleRegularExpression.match(leftSide).hasMatch())
    {
        return MegaIgnoreRule::RuleType::InvalidRule;
    }

    // Check if type rule
    const static QRegularExpression typeRuleRegEx{ QLatin1String(::RIGHT_SIDE_TYPE_RULE_REG_EX) };
    if (typeRuleRegEx.match(rightSide).hasMatch())
    {
        return MegaIgnoreRule::RuleType::ExtensionRule;
    }
    return MegaIgnoreRule::NameRule;
}

QList<std::shared_ptr<MegaIgnoreNameRule> > MegaIgnoreManager::getNameRules() const
{
    QList<std::shared_ptr<MegaIgnoreNameRule>> rules;
    foreach(auto & rule, mRules)
    {
        if (rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::NameRule)
        {
            auto nameRule = convert<MegaIgnoreNameRule>(rule);
            rules.append(nameRule);
        }
    }

    return rules;
}

QStringList MegaIgnoreManager::getExcludedExtensions() const
{
    QStringList extensions;
    foreach(auto & rule, mRules)
    {
        if (rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::ExtensionRule)
        {
            auto extensionRule = convert<MegaIgnoreExtensionRule>(rule);
            if (extensionRule)
            {
                extensions.append(extensionRule->extension());
            }
        }
    }
    return extensions;
}

void MegaIgnoreManager::enableExtensions(bool state)
{
    foreach(auto & rule, mRules)
    {
        if (rule->isValid() && rule->ruleType() == MegaIgnoreRule::RuleType::ExtensionRule)
        {
            rule->setCommented(!state);
        }
    }
}

MegaIgnoreManager::ApplyChangesError MegaIgnoreManager::applyChanges(bool updateExtensionRules, const QStringList& updatedExtensions)
{
    ApplyChangesError result(ApplyChangesError::NoUpdateNeeded);
    QStringList rules;
    foreach(auto & rule, mRules)
    {
        if (!rule->isDeleted())
        {
            if (!rule->isValid())
            {
                rule->setCommented(true);
            }
            if (rule->ruleType() == MegaIgnoreRule::RuleType::ExtensionRule && updateExtensionRules)
            {
                continue;
            }
            auto ruleAsText(rule->getModifiedRule());
            if (!ruleAsText.isEmpty())
            {
                rules.append(ruleAsText);
            }
        }

        if (rule->isDirty() || updateExtensionRules)
        {
            result = ApplyChangesError::Ok;
        }
    }
    if (updateExtensionRules)
    {
        for (const auto& extension : updatedExtensions)
        {
            const auto trimmed = extension.trimmed();
            if (mExtensionRules.contains(trimmed))
            {
                rules.append(mExtensionRules.value(trimmed)->getModifiedRule());
            }
            else
            {
                const MegaIgnoreExtensionRule extensionRule(MegaIgnoreNameRule::Class::Exclude, trimmed);
                rules.append(extensionRule.getModifiedRule());
            }
        }
    }
    if (result == ApplyChangesError::Ok)
    {
        QFile ignore(mOutputMegaIgnoreFile);

        if (ignore.open(QIODevice::WriteOnly))
        {
            QTextStream out(&ignore);
            out << rules.join(QLatin1String("\n"));

            ignore.close();
        }
        else
        {
            result = ApplyChangesError::NoWritePermission;
        }
    }

    return result;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::addIgnoreSymLinksRule()
{
    if (!mIgnoreSymLinkRule)
    {
        auto rule = std::make_shared<MegaIgnoreNameRule>(QLatin1String("*"), MegaIgnoreNameRule::Class::Exclude, MegaIgnoreNameRule::Target::s);
        addRule(rule);
    }
    else if (mIgnoreSymLinkRule->isCommented())
    {
        mIgnoreSymLinkRule->setCommented(false);
    }

    return mIgnoreSymLinkRule;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::addIgnoreSymLinkRule(const QString& pattern)
{
    auto rule = std::make_shared<MegaIgnoreNameRule>(pattern, MegaIgnoreNameRule::Class::Exclude, MegaIgnoreNameRule::Target::s);
    addRule(rule);
    return rule;
}

std::shared_ptr<MegaIgnoreNameRule> MegaIgnoreManager::addNameRule(MegaIgnoreNameRule::Class classType, const QString& pattern, MegaIgnoreNameRule::Target targetType)
{
    auto rule = std::make_shared<MegaIgnoreNameRule>(pattern, classType, targetType);
    addRule(rule);
    return rule;
}

void MegaIgnoreManager::setOutputIgnorePath(const QString& outputPath)
{
    mOutputMegaIgnoreFile = outputPath;
}

bool MegaIgnoreManager::hasChanged() const
{
    std::unique_ptr<char[]> crc(MegaSyncApp->getMegaApi()->getCRC(mMegaIgnoreFile.toStdString().c_str()));
    auto IgnoreCRC = QString::fromUtf8(crc.get());
    return mIgnoreCRC.compare(IgnoreCRC) != 0;
}

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
    mRuleType = RuleType::NameRule;

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
                    mClass = chr == QLatin1String("-") ? Class::Exclude : Class::Include;
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

MegaIgnoreNameRule::Target MegaIgnoreNameRule::getDisplayTarget()
{
    if (mTarget == MegaIgnoreNameRule::Target::f || mTarget == MegaIgnoreNameRule::Target::d)
    {
        return mTarget;
    }
    return mPattern.contains(QLatin1String(".")) ? MegaIgnoreNameRule::Target::f : MegaIgnoreNameRule::Target::d;
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

        rule.append(mClass == Class::Exclude ? QLatin1String("-") : QLatin1String("+"));
        if (mTarget != Target::None)
        {
            EnumConversions<Target> convertEnum;
            rule.append(convertEnum.getString(mTarget));
        }
        if (mType != Type::None)
        {
            EnumConversions<Type> convertEnum;
            rule.append(convertEnum.getString(mType));
        }
        if (mStrategy != Strategy::None)
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
        mWildCardType = MegaIgnoreNameRule::WildCardType::Equal;
    }
    else if (asteriskCount > 1)
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::Conatains;
    }
    else if (rightSidePart.startsWith(QLatin1String("*")))
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::EndsWith;
    }
    else
    {
        mWildCardType = MegaIgnoreNameRule::WildCardType::StartsWith;
    }
}

////////////////MEGA IGNORE EXTENSION RULE
MegaIgnoreExtensionRule::MegaIgnoreExtensionRule(const QString& rule, bool isCommented)
    : MegaIgnoreNameRule(rule, isCommented)
{
    mRuleType = RuleType::ExtensionRule;
    auto extensionSplitter(mPattern.split(QLatin1String(".")));
    QRegExp captureExtension{ QLatin1String(::CAPTURE_EXTENSION_REG_EX) };
    if (captureExtension.indexIn(rule) != -1)
    {
        mExtension = captureExtension.cap(1); // Capture extension
    }
}

MegaIgnoreExtensionRule::MegaIgnoreExtensionRule(Class classType, const QString& extension) :MegaIgnoreNameRule(QString::fromUtf8("*.") + extension, classType)
{
    mRuleType = RuleType::ExtensionRule;
    mExtension = extension;
}

const QString& MegaIgnoreExtensionRule::extension() const
{
    return mExtension;
}

////////////////MEGA IGNORE SIZE RULE
const QString MegaIgnoreSizeRule::LOW_STRING = QLatin1String("exclude-smaller");
const QString MegaIgnoreSizeRule::HIGH_STRING = QLatin1String("exclude-larger");

MegaIgnoreSizeRule::MegaIgnoreSizeRule(const QString& rule, bool isCommented)
    :MegaIgnoreRule(rule, isCommented)
{
    mRuleType = RuleType::SizeRule;

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
            mThreshold = Threshold::High;
        }
        else if (type == LOW_STRING)
        {
            mThreshold = Threshold::Low;
        }

        auto size(ruleSplitted.at(1));

        QRegularExpression rx(QLatin1String("[0-9]+"));
        QRegularExpressionMatch match = rx.match(size);
        if (match.hasMatch())
        {
            auto value = match.captured(0);
            mValue = value.toUInt();
            auto unit = size.remove(value);
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
    mRuleType = RuleType::SizeRule;
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
        rule.append(mThreshold == Threshold::Low ? LOW_STRING : HIGH_STRING);
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

QStringList MegaIgnoreSizeRule::getUnitsForDisplay()
{
    return QStringList() << QApplication::translate("Sizes", "B")
        << QApplication::translate("Sizes", "KB")
        << QApplication::translate("Sizes", "MB")
        << QApplication::translate("Sizes", "GB");
}

void MegaIgnoreSizeRule::setValue(uint64_t newValue)
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
