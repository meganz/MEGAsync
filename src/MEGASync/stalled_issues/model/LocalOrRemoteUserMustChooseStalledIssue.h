#ifndef LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H
#define LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H

#include <StalledIssue.h>

class MegaUploader;

class LocalOrRemoteUserMustChooseStalledIssue : public StalledIssue
{
public:
    LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue);

    void solveIssue(bool) override;
    bool isSolvable() const;

    void endFillingIssue() override;

    void chooseLocalSide();
    void chooseRemoteSide();
    void chooseLastMTimeSide();

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
