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

    bool autoSolveIssue() override;
    bool isSolvable() const override;

    void fillIssue(const mega::MegaSyncStall *stall) override;
    void endFillingIssue() override;

    void chooseLocalSide();
    void chooseRemoteSide();
    void chooseLastMTimeSide();

    bool UIShowFileAttributes() const override;

    enum class ChosenSide
    {
        None = 0,
        Remote,
        Local
    };

    ChosenSide getChosenSide() const;

private:
    MegaUploader* mUploader;
    ChosenSide mChosenSide = ChosenSide::None;

};

#endif // LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H
