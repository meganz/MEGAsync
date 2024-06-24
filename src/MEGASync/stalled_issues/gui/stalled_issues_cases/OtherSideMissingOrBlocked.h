#ifndef OTHERSIDEMISSINGORBLOCKED_H
#define OTHERSIDEMISSINGORBLOCKED_H

#include <StalledIssueBaseDelegateWidget.h>

namespace Ui {
class OtherSideMissingOrBlocked;
}

class OtherSideMissingOrBlocked : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit OtherSideMissingOrBlocked(QWidget *parent = nullptr);
    ~OtherSideMissingOrBlocked();

protected:
    void refreshUi();

private:
    Ui::OtherSideMissingOrBlocked *ui;
};

#endif // OTHERSIDEMISSINGORBLOCKED_H
