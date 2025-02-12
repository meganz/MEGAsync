#ifndef MEGACATCHREPORTER_H
#define MEGACATCHREPORTER_H

#include "MegaCatchReporterUtilities.h"

class MegaCatchReporter: public Catch::StreamingReporterBase<MegaCatchReporter>
{
public:
    MegaCatchReporter(const Catch::ReporterConfig& config);
    virtual ~MegaCatchReporter() = default;

    static std::string getDescription();
    static std::set<Catch::Verbosity> getSupportedVerbosities();

    void assertionStarting(Catch::AssertionInfo const&) override;
    bool assertionEnded(Catch::AssertionStats const& assertionStats) override;

    void sectionStarting(Catch::SectionInfo const& sectionInfo) override;
    void sectionEnded(Catch::SectionStats const& sectionStats) override;

    void testCaseStarting(Catch::TestCaseInfo const& testCaseInfo) override;
    void testCaseEnded(Catch::TestCaseStats const& testCaseStats) override;

    void testGroupStarting(Catch::GroupInfo const& groupInfo) override;
    void testGroupEnded(Catch::TestGroupStats const& testGroupStats) override;

    void testRunStarting(Catch::TestRunInfo const& testRunInfo) override;
    void testRunEnded(Catch::TestRunStats const& testRunStats) override;

private:
    void printLineDelimiter(int lines = 1);
    void printResultStatus(const Catch::Counts& counts);
    void printTotals(const std::string& title, const Catch::Counts& counts);
    std::string getResultDescription(const Catch::Counts& counts);
    void printFailInformation();

    std::string printableTabs();
    std::string printableTabs(const size_t count);
    std::string completeLine(const char delimiter, int size);
    std::string getClassName(const Catch::TestCaseInfo& info);
    std::string getTestName(const std::string& rawName);

    void printFailSourceInformation(const AssertionFailureData& data);
    void printAdditionalInfo(const std::string& addicionalInfo,
                             const std::vector<Catch::MessageInfo>& infoMessages);
    void printFailureDetails(const AssertionFailureData& data);
    void printDebugInfo(const AssertionFailureData& data);

    bool isInPreviousStack(const std::string& currentSection);
    void updatePreviousStack(const std::string& currentSection);

    ResultHandler mResultHandler;

    const int mLineCharSize = 70;
    const size_t mMaxLineSize = 63;

    size_t mMaxSectionLevelToPrint = 3;

    std::string mCurrentTestInternalContents = "";
    std::string mCurrentTestClass = "";
    const std::string mSingleTab = "  ";
    std::vector<AssertionFailureData> mFailInformation;
    bool mCrashDetected = false;
    std::vector<std::string> mPreviousFullSectionStack;
};

CATCH_REGISTER_REPORTER("mega", MegaCatchReporter);

#endif // MEGACATCHREPORTER_H
