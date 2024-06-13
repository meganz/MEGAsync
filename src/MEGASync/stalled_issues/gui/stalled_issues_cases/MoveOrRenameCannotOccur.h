#ifndef MOVEORRENAMECANNOTOCCUR_H
#define MOVEORRENAMECANNOTOCCUR_H

#include <StalledIssueBaseDelegateWidget.h>

namespace Ui
{
class MoveOrRenameCannotOccur;
}

class MoveOrRenameCannotOccur : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit MoveOrRenameCannotOccur(QWidget* parent = nullptr);
    ~MoveOrRenameCannotOccur();

    void refreshUi() override;

private slots:
    void onLocalButtonClicked();
    void onRemoteButtonClicked();

private:
    Ui::MoveOrRenameCannotOccur* ui;
};

#endif // MOVEORRENAMECANNOTOCCUR_H
