#ifndef LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H
#define LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H

#include <StalledIssue.h>
#include <TransfersModel.h>

class MegaUploader;

class LocalOrRemoteUserMustChooseStalledIssue : public StalledIssue
{
public:
    LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue);
    ~LocalOrRemoteUserMustChooseStalledIssue();

    void setIsSolved(SolveType type) override;
    bool autoSolveIssue() override;
    bool isAutoSolvable() const override;

    void fillIssue(const mega::MegaSyncStall *stall) override;
    void endFillingIssue() override;

    bool chooseLocalSide();
    bool chooseRemoteSide();
    bool chooseLastMTimeSide();
    bool chooseBothSides(QStringList *namesUsed);

    bool UIShowFileAttributes() const override;

    enum class ChosenSide
    {
        NONE = 0,
        REMOTE,
        LOCAL,
        BOTH
    };

    ChosenSide getChosenSide() const;
    ChosenSide lastModifiedSide() const;

private:
    MegaUploader* mUploader;
    ChosenSide mChosenSide = ChosenSide::NONE;
    QString mNewName;
};

#endif // LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H
