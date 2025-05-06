#include "MegaCatchReporter.h"

MegaCatchReporter::MegaCatchReporter(Catch::ReporterConfig&& config):
    Catch::StreamingReporterBase(std::move(config))
{
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

void MegaCatchReporter::assertionEnded(const Catch::AssertionStats& assertionStats)
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
        m_stream << std::endl << mCurrentTestClass << std::endl;
    }

    std::string lineContent = printableTabs() + getTestName(testCaseInfo.name);
    m_stream << lineContent
             << completeLine(' ', mLineCharSize - static_cast<int>(lineContent.size()));
}

void MegaCatchReporter::testCaseEnded(const Catch::TestCaseStats& testCaseStats)
{
    StreamingReporterBase::testCaseEnded(testCaseStats);

    if (mCrashDetected)
    {
        m_colour->guardColour(Catch::Colour::BrightRed);
        m_stream << "Crash" << std::endl;
    }
    else
    {
        printResultStatus(testCaseStats.totals.assertions);
    }

    m_colour->guardColour(Catch::Colour::Grey);
    m_stream << mCurrentTestInternalContents;
    mCurrentTestInternalContents = "";
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
    m_stream << "Finished test run " << testRunStats.runInfo.name << std::endl;
    printTotals("test cases", testRunStats.totals.testCases);
    printTotals("assertions", testRunStats.totals.assertions);
    printLineDelimiter(2);

    if (!mFailInformation.empty() && m_config->verbosity() != Catch::Verbosity::Quiet)
    {
        m_stream << "Details about errors :" << std::endl;
        printLineDelimiter();
        printFailInformation();
    }
}

void MegaCatchReporter::printLineDelimiter(int lines)
{
    m_stream << std::endl;
    for (int i = 0; i < lines; ++i)
    {
        m_stream << completeLine('-', mLineCharSize + 20) << std::endl;
    }
    m_stream << std::endl;
}

void MegaCatchReporter::printResultStatus(const Catch::Counts& counts)
{
    if (counts.failed > 0)
    {
        m_colour->guardColour(Catch::Colour::Error);
        m_stream << "Failed";
    }
    else
    {
        m_colour->guardColour(Catch::Colour::Success);
        m_stream << "OK";
    }
    m_stream << " (" << getResultDescription(counts) << ")" << std::endl;
}

void MegaCatchReporter::printTotals(const std::string& title, const Catch::Counts& counts)
{
    m_stream << counts.total() << " " << title << " in total. ";
    m_stream << getResultDescription(counts) << std::endl;
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
        m_colour->guardColour(Catch::Colour::FileName);
        m_stream << "\tin " << result.getSourceInfo() << " : ";
        auto resultData = mResultHandler.getData(result.getResultType());

        m_colour->guardColour(resultData.mColour);
        m_stream << resultData.mTitle;
        m_stream << std::endl;

        printAdditionalInfo(resultData.mAdditionalInfo, failData.mStats.infoMessages);

        printFailureDetails(failData);

        m_colour->guardColour(Catch::Colour::None);
        // printDebugInfo(failData);

        m_stream << std::endl << failDelimiter << std::endl << std::endl;
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
    return std::string(info.className);
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
    m_stream << data.mTestCaseName << std::endl;
    for (size_t i = 0; i < data.mSectionStack.size(); ++i)
    {
        auto currentSection = data.mSectionStack.at(i);
        auto sectionName = (i == 0) ? getTestName(currentSection.name) : currentSection.name;
        m_stream << printableTabs(i + 1) << sectionName;
        m_stream << " (in " << currentSection.lineInfo << ")" << std::endl;
    }
}

void MegaCatchReporter::printAdditionalInfo(const std::string& addicionalInfo,
                                            const std::vector<Catch::MessageInfo>& infoMessages)
{
    m_colour->guardColour(Catch::Colour::None);
    if (addicionalInfo != "")
    {
        m_stream << addicionalInfo;
    }

    if (!infoMessages.empty())
    {
        for (auto message: infoMessages)
        {
            m_stream << message.message << std::endl;
        }
    }
    else
    {
        m_stream << std::endl;
    }
}

void MegaCatchReporter::printFailureDetails(const AssertionFailureData& data)
{
    auto result = data.mStats.assertionResult;
    if (result.hasExpression())
    {
        m_colour->guardColour(Catch::Colour::OriginalExpression);
        auto originalExpression = data.mStats.assertionResult.getExpressionInMacro();
        m_stream << originalExpression << std::endl;
        if (data.mExpandedExpression != "" && data.mExpandedExpression != originalExpression &&
            data.mExpandedExpression != data.mStats.assertionResult.getExpression())
        {
            m_stream << "Expanded to :" << std::endl;
            m_colour->guardColour(Catch::Colour::ReconstructedExpression);
            m_stream << data.mExpandedExpression << std::endl;
        }
    }
}

void MegaCatchReporter::printDebugInfo(const AssertionFailureData& data)
{
    m_colour->guardColour(Catch::Colour::None);
    m_stream << "*** DEBUG INFO ***" << std::endl;
    m_stream << "Result type : " << data.mStats.assertionResult.getResultType() << std::endl;
    m_stream << "InfoMsgs (" << data.mStats.infoMessages.size() << ")" << std::endl;
    for (auto itMsg: data.mStats.infoMessages)
    {
        m_stream << "\tmacroName : " << itMsg.macroName << std::endl;
        m_stream << "\ttype : " << itMsg.type << std::endl;
        m_stream << "\tlineInfo : " << itMsg.lineInfo << std::endl;
        m_stream << "\tsequence : " << itMsg.sequence << std::endl;
        m_stream << "\tContent : " << itMsg.message << std::endl;
    }
    m_stream << "Test case : " << data.mTestCaseName << std::endl;
    m_stream << "sectionStack :" << std::endl;
    for (auto section: data.mSectionStack)
    {
        m_stream << "\tname : " << section.name << std::endl;
        m_stream << "\tlineinfo : " << section.lineInfo << std::endl;
    }
    m_stream << "******************" << std::endl;
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
