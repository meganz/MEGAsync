#ifndef LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H
#define LOCALORREMOTEUSERMUSTCHOOSESTALLEDISSUE_H

#include <StalledIssue.h>

class MegaUploader;

class LocalOrRemoteUserMustChooseStalledIssue : public StalledIssue
{
public:
    LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue);

    void solveIssue(bool autosolve) override;
    void chooseLocalSide();
    void chooseRemoteSide();

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
