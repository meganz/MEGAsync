#include "MegaCatchReporter.h"

MegaCatchReporter::MegaCatchReporter(const Catch::ReporterConfig& config):
    Catch::StreamingReporterBase<MegaCatchReporter>(config)
{
    m_reporterPrefs.shouldRedirectStdOut = true;
    mMaxSectionLevelToPrint = (config.fullConfig()->verbosity() == Catch::Verbosity::High) ? 3 : 1;
}

std::string MegaCatchReporter::getDescription()
{
    return "Console reporter with results hierarchically organized";
}

std::set<Catch::Verbosity> MegaCatchReporter::getSupportedVerbosities()
{
    return {Catch::Verbosity::Quiet, Catch::Verbosity::Normal, Catch::Verbosity::High};
}

void MegaCatchReporter::assertionStarting(const Catch::AssertionInfo&) {}

bool MegaCatchReporter::assertionEnded(const Catch::AssertionStats& assertionStats)
{
    const auto& result = assertionStats.assertionResult;
    if (!result.isOk())
    {
        AssertionFailureData data(assertionStats,
                                  m_sectionStack,
                                  mCurrentTestClass,
                                  result.getExpandedExpression());
        mFailInformation.push_back(data);
        if (result.getResultType() == Catch::ResultWas::FatalErrorCondition)
        {
            mCrashDetected = true;
        }
    }
    return true;
}

void MegaCatchReporter::sectionStarting(const Catch::SectionInfo& sectionInfo)
{
    StreamingReporterBase::sectionStarting(sectionInfo);

    if (!isInPreviousStack(sectionInfo.name))
    {
        bool isMethodName = (m_sectionStack.size() == 1);
        bool isNotTooDeep = (m_sectionStack.size() <= mMaxSectionLevelToPrint);
        if (!isMethodName && isNotTooDeep) // Method name already handled in test case.
        {
            std::string sectionLine = printableTabs() + sectionInfo.name;
            if (sectionLine.size() > mMaxLineSize)
            {
                sectionLine = sectionLine.substr(0, mMaxLineSize);
                sectionLine += "...";
            }
            mCurrentTestInternalContents += sectionLine + "\n";
        }
    }
    updatePreviousStack(sectionInfo.name);
}

void MegaCatchReporter::sectionEnded(const Catch::SectionStats& sectionStats)
{
    StreamingReporterBase::sectionEnded(sectionStats);
}

void MegaCatchReporter::testCaseStarting(const Catch::TestCaseInfo& testCaseInfo)
{
    std::string startingTestClass = getClassName(testCaseInfo);
    bool startingNewTestClass =
        (startingTestClass != mCurrentTestClass) || (mCurrentTestClass.empty());
    if (startingNewTestClass)
    {
        mCurrentTestClass = startingTestClass;
        stream << std::endl << mCurrentTestClass << std::endl;
    }

    std::string lineContent = printableTabs() + getTestName(testCaseInfo.name);
    stream << lineContent
           << completeLine(' ', mLineCharSize - static_cast<int>(lineContent.size()));
}

void MegaCatchReporter::testCaseEnded(const Catch::TestCaseStats& testCaseStats)
{
    StreamingReporterBase::testCaseEnded(testCaseStats);

    if (mCrashDetected)
    {
        Catch::Colour colourGuard(Catch::Colour::BrightRed);
        stream << "Crash" << std::endl;
    }
    else
    {
        printResultStatus(testCaseStats.totals.assertions);
    }

    Catch::Colour colourGuard(Catch::Colour::Grey);
    stream << mCurrentTestInternalContents;
    mCurrentTestInternalContents = "";
}

void MegaCatchReporter::testGroupStarting(const Catch::GroupInfo& groupInfo)
{
    StreamingReporterBase::testGroupStarting(groupInfo);
}

void MegaCatchReporter::testGroupEnded(const Catch::TestGroupStats& testGroupStats)
{
    StreamingReporterBase::testGroupEnded(testGroupStats);
}

void MegaCatchReporter::testRunStarting(const Catch::TestRunInfo& testRunInfo)
{
    printLineDelimiter();
    StreamingReporterBase::testRunStarting(testRunInfo);
}

void MegaCatchReporter::testRunEnded(const Catch::TestRunStats& testRunStats)
{
    StreamingReporterBase::testRunEnded(testRunStats);
    printLineDelimiter();
    stream << "Finished test run " << testRunStats.runInfo.name << std::endl;
    printTotals("test cases", testRunStats.totals.testCases);
    printTotals("assertions", testRunStats.totals.assertions);
    printLineDelimiter(2);

    if (!mFailInformation.empty() && m_config->verbosity() != Catch::Verbosity::Quiet)
    {
        stream << "Details about errors :" << std::endl;
        printLineDelimiter();
        printFailInformation();
    }
}

void MegaCatchReporter::printLineDelimiter(int lines)
{
    stream << std::endl;
    for (int i = 0; i < lines; ++i)
    {
        stream << completeLine('-', mLineCharSize + 20) << std::endl;
    }
    stream << std::endl;
}

void MegaCatchReporter::printResultStatus(const Catch::Counts& counts)
{
    if (counts.failed > 0)
    {
        Catch::Colour colourGuard(Catch::Colour::Error);
        stream << "Failed";
    }
    else
    {
        Catch::Colour colourGuard(Catch::Colour::Success);
        stream << "OK";
    }
    stream << " (" << getResultDescription(counts) << ")" << std::endl;
}

void MegaCatchReporter::printTotals(const std::string& title, const Catch::Counts& counts)
{
    stream << counts.total() << " " << title << " in total. ";
    stream << getResultDescription(counts) << std::endl;
}

std::string MegaCatchReporter::getResultDescription(const Catch::Counts& counts)
{
    std::string description;
    if (counts.passed > 0)
    {
        description += std::to_string(counts.passed) + " passed";
    }
    if (counts.failed > 0)
    {
        if (description != "")
        {
            description += ", ";
        }
        description += std::to_string(counts.failed) + " failed";
    }
    return description;
}

void MegaCatchReporter::printFailInformation()
{
    const std::string failDelimiter = "-----------------";
    for (auto failData: mFailInformation)
    {
        auto result = failData.mStats.assertionResult;

        printFailSourceInformation(failData);
        Catch::Colour colourGuard1(Catch::Colour::FileName);
        stream << "\tin " << result.getSourceInfo() << " : ";
        auto resultData = mResultHandler.getData(result.getResultType());

        Catch::Colour colourGuard12(resultData.mColour);
        stream << resultData.mTitle;
        stream << std::endl;

        printAdditionalInfo(resultData.mAdditionalInfo, failData.mStats.infoMessages);

        printFailureDetails(failData);

        Catch::Colour colourGuard4(Catch::Colour::None);
        // printDebugInfo(failData);

        stream << std::endl << failDelimiter << std::endl << std::endl;
    }
}

std::string MegaCatchReporter::printableTabs()
{
    return printableTabs(m_sectionStack.size() + 1);
}

std::string MegaCatchReporter::printableTabs(const size_t count)
{
    std::string tabs;
    for (size_t i = 0; i < count; ++i)
    {
        tabs += mSingleTab;
    }
    return tabs;
}

std::string MegaCatchReporter::completeLine(const char delimiter, int size)
{
    std::string line;
    for (int i = 0; i < size; ++i)
    {
        line += delimiter;
    }
    return line;
}

std::string MegaCatchReporter::getClassName(const Catch::TestCaseInfo& info)
{
    if (info.className.empty())
    {
        const std::string classTag = "class ";
        if (info.name.find(classTag) == 0)
        {
            auto classNameSize = info.name.find(" ", classTag.size());
            return info.name.substr(classTag.size(), classNameSize - classTag.size());
        }
        return "Unnamed Class";
    }
    return info.className;
}

std::string MegaCatchReporter::getTestName(const std::string& rawName)
{
    const std::string classTag = "class ";
    if (rawName.find(classTag) == 0)
    {
        auto classNameSize = rawName.find(" ", classTag.size());
        auto classNameWithWhitespacesSize = rawName.find_first_not_of(" ", classNameSize);
        return rawName.substr(classNameWithWhitespacesSize);
    }
    return rawName;
}

void MegaCatchReporter::printFailSourceInformation(const AssertionFailureData& data)
{
    stream << data.mTestCaseName << std::endl;
    for (size_t i = 0; i < data.mSectionStack.size(); ++i)
    {
        auto currentSection = data.mSectionStack.at(i);
        auto sectionName = (i == 0) ? getTestName(currentSection.name) : currentSection.name;
        stream << printableTabs(i + 1) << sectionName;
        stream << " (in " << currentSection.lineInfo << ")" << std::endl;
    }
}

void MegaCatchReporter::printAdditionalInfo(const std::string& addicionalInfo,
                                            const std::vector<Catch::MessageInfo>& infoMessages)
{
    Catch::Colour colourGuard13(Catch::Colour::None);
    if (addicionalInfo != "")
    {
        stream << addicionalInfo;
    }

    if (!infoMessages.empty())
    {
        for (auto message: infoMessages)
        {
            stream << message.message << std::endl;
        }
    }
    else
    {
        stream << std::endl;
    }
}

void MegaCatchReporter::printFailureDetails(const AssertionFailureData& data)
{
    auto result = data.mStats.assertionResult;
    if (result.hasExpression())
    {
        Catch::Colour colourGuard2(Catch::Colour::OriginalExpression);
        auto originalExpression = data.mStats.assertionResult.getExpressionInMacro();
        stream << originalExpression << std::endl;
        if (data.mExpandedExpression != "" && data.mExpandedExpression != originalExpression &&
            data.mExpandedExpression != data.mStats.assertionResult.getExpression())
        {
            stream << "Expanded to :" << std::endl;
            Catch::Colour colourGuard3(Catch::Colour::ReconstructedExpression);
            stream << data.mExpandedExpression << std::endl;
        }
    }
}

void MegaCatchReporter::printDebugInfo(const AssertionFailureData& data)
{
    Catch::Colour colourGuard3(Catch::Colour::None);
    stream << "*** DEBUG INFO ***" << std::endl;
    stream << "Result type : " << data.mStats.assertionResult.getResultType() << std::endl;
    stream << "InfoMsgs (" << data.mStats.infoMessages.size() << ")" << std::endl;
    for (auto itMsg: data.mStats.infoMessages)
    {
        stream << "\tmacroName : " << itMsg.macroName << std::endl;
        stream << "\ttype : " << itMsg.type << std::endl;
        stream << "\tlineInfo : " << itMsg.lineInfo << std::endl;
        stream << "\tsequence : " << itMsg.sequence << std::endl;
        stream << "\tContent : " << itMsg.message << std::endl;
    }
    stream << "Test case : " << data.mTestCaseName << std::endl;
    stream << "sectionStack :" << std::endl;
    for (auto section: data.mSectionStack)
    {
        stream << "\tname : " << section.name << std::endl;
        stream << "\tlineinfo : " << section.lineInfo << std::endl;
    }
    stream << "******************" << std::endl;
}

bool MegaCatchReporter::isInPreviousStack(const std::string& currentSection)
{
    auto it = std::find(mPreviousFullSectionStack.begin(),
                        mPreviousFullSectionStack.end(),
                        currentSection);
    return (it != mPreviousFullSectionStack.end());
}

void MegaCatchReporter::updatePreviousStack(const std::string& currentSection)
{
    if (mPreviousFullSectionStack.size() < m_sectionStack.size())
    {
        mPreviousFullSectionStack.push_back(currentSection);
    }
    else if (mPreviousFullSectionStack.size() == m_sectionStack.size())
    {
        mPreviousFullSectionStack.back() = currentSection;
    }
    else
    {
        for (size_t i = 0; i < m_sectionStack.size(); ++i)
        {
            if (mPreviousFullSectionStack[i] != m_sectionStack[i].name)
            {
                mPreviousFullSectionStack.erase(mPreviousFullSectionStack.begin() + i,
                                                mPreviousFullSectionStack.end());
            }
        }
    }
}
