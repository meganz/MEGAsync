#include "StalledIssueHeader.h"
#include "ui_StalledIssueHeader.h"

#include "Utilities.h"

StalledIssueHeader::StalledIssueHeader(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader)
{
    ui->setupUi(this);
}

StalledIssueHeader::~StalledIssueHeader()
{
    delete ui;
}

void StalledIssueHeader::refreshUi()
{

}

//void StalledIssueHeader::setFilename(const QString &name)
//{
//    auto fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
//                                              name, QLatin1Literal(":/images/drag_")));
//    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));
//}
