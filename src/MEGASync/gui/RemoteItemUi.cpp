#include "RemoteItemUi.h"
#include "ui_RemoteItemUi.h"

RemoteItemUi::RemoteItemUi(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::RemoteItemUi)
{
    ui->setupUi(this);
    connect(ui->bAdd, &QPushButton::clicked, this, [this](){
        emit addClicked(mega::INVALID_HANDLE);
    });
    connect(ui->bDelete, &QPushButton::clicked, this, &RemoteItemUi::deleteClicked);
    connect(ui->bPermissions, &QPushButton::clicked, this, &RemoteItemUi::permissionsClicked);
}

RemoteItemUi::~RemoteItemUi()
{
    delete ui;
}

void RemoteItemUi::initView(QTableView *newView)
{
    newView->setParent(this);
    newView->setObjectName(QString::fromUtf8("tableViewReplaced"));
    newView->setStyleSheet(ui->tableView->styleSheet());
    setTableViewProperties(newView);

    auto oldLayoutItem = ui->verticalLayout->replaceWidget(ui->tableView, newView);
    delete oldLayoutItem;
    delete ui->tableView;
    ui->tableView = newView;
}

void RemoteItemUi::setUsePermissions(const bool use)
{
    ui->bPermissions->setVisible(use);
}

QTableView *RemoteItemUi::getView()
{
    return ui->tableView;
}

void RemoteItemUi::setTableViewProperties(QTableView *view) const
{
    view->setFrameShape(QFrame::NoFrame);
    view->setEditTriggers(QAbstractItemView::AllEditTriggers);
    view->setAlternatingRowColors(false);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setTextElideMode(Qt::ElideMiddle);
    view->setShowGrid(false);
    view->setSortingEnabled(true);
    view->setCornerButtonEnabled(false);
    view->horizontalHeader()->setDefaultSectionSize(60);
    view->horizontalHeader()->setHighlightSections(false);
    view->verticalHeader()->setVisible(false);
    view->verticalHeader()->setMinimumSectionSize(24);
    view->verticalHeader()->setDefaultSectionSize(24);
}
