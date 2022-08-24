#ifndef STALLEDISSUECHOOSETITLE_H
#define STALLEDISSUECHOOSETITLE_H

#include "gui/StalledIssueActionTitle.h"

class StalledIssueChooseTitle : public StalledIssueActionTitle
{
public:
    StalledIssueChooseTitle(QWidget *parent = nullptr);

    void showIcon() override;
};

#endif // STALLEDISSUECHOOSETITLE_H
