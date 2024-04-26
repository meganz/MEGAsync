#ifndef MOVEORRENAMECANNOTOCCUR_H
#define MOVEORRENAMECANNOTOCCUR_H

#include <StalledIssueBaseDelegateWidget.h>

#include <QWidget>

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

private:
    Ui::MoveOrRenameCannotOccur* ui;
};

#endif // MOVEORRENAMECANNOTOCCUR_H
