#include "StalledIssueHeader.h"

#include "Utilities.h"

const int StalledIssueHeader::ARROW_INDENT = 6 + 16; //Left margin + arrow;
const int StalledIssueHeader::ICON_INDENT = 8 + 48; // fileIcon + spacer;
const int StalledIssueHeader::BODY_INDENT = StalledIssueHeader::ARROW_INDENT + StalledIssueHeader::ICON_INDENT; // full indent;

StalledIssueHeader::StalledIssueHeader(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueHeader)
{
    ui->setupUi(this);
    ui->actionButton->hide();
    connect(ui->actionButton, &QPushButton::clicked, this, &StalledIssueHeader::actionClicked);
}

StalledIssueHeader::~StalledIssueHeader()
{
    delete ui;
}

void StalledIssueHeader::expand(bool state)
{
    auto arrowIcon = Utilities::getCachedPixmap(state ? QLatin1Literal(":/images/node_selector/Icon-Small-Arrow-Down.png") :  QLatin1Literal(":/images/node_selector/Icon-Small-Arrow-Left.png"));
    ui->arrow->setPixmap(arrowIcon.pixmap(ui->arrow->size()));
}

void StalledIssueHeader::showAction()
{
    ui->actionButton->setVisible(true);
}

void StalledIssueHeader::refreshUi()
{
    QIcon fileTypeIcon;

    auto splittedFile = getData().getFileName().split(QString::fromUtf8("."));
    if(splittedFile.size() != 1)
    {
        fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                      getData().getFileName(), QLatin1Literal(":/images/drag_")));
    }
    else
    {
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/color_folder.png"));
    }

    ui->fileTypeIcon->setPixmap(fileTypeIcon.pixmap(ui->fileTypeIcon->size()));

    refreshCaseUi();
}
