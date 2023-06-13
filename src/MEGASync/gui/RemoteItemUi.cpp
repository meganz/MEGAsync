#include "RemoteItemUi.h"
#include "ui_RemoteItemUi.h"

#ifndef Q_OS_WIN
#include <MegaApplication.h>
#include <DialogOpener.h>
#include <PermissionsDialog.h>
#endif

RemoteItemUi::RemoteItemUi(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteItemUi)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    setUsePermissions(false);
#else
    connect(ui->bPermissions, &QPushButton::clicked, this, &RemoteItemUi::permissionsClicked);
#endif

#ifdef Q_OS_MACOS
    ui->tableSegementedControl->configureTableSegment();
    connect(ui->tableSegementedControl, &QSegmentedControl::addButtonClicked,
            this, [this](){
        emit addClicked(mega::INVALID_HANDLE);
    });
    connect(ui->tableSegementedControl, &QSegmentedControl::removeButtonClicked,
            this,  &RemoteItemUi::deleteClicked);
#else
    connect(ui->bAdd, &QPushButton::clicked, this, [this](){
        emit addClicked(mega::INVALID_HANDLE);
    });
    connect(ui->bDelete, &QPushButton::clicked, this, &RemoteItemUi::deleteClicked);
#endif
}

RemoteItemUi::~RemoteItemUi()
{
    delete ui;
}

void RemoteItemUi::setTitle(const QString &title)
{
#ifdef Q_OS_MACOS
    ui->title->setText(title);
#else
    ui->groupBox->setTitle(title);
#endif
}

void RemoteItemUi::initView(QTableView *newView)
{
    newView->setParent(this);
    newView->setObjectName(QString::fromUtf8("tableViewReplaced"));
    newView->setStyleSheet(ui->tableView->styleSheet());
    setTableViewProperties(newView);

    auto oldLayoutItem = ui->tableLayout->replaceWidget(ui->tableView, newView);
    delete oldLayoutItem;
    delete ui->tableView;
    ui->tableView = newView;
}

void RemoteItemUi::setUsePermissions(const bool use)
{
    ui->bPermissions->setVisible(use);

    if(ui->bPermissions->isHidden()
        #ifndef Q_OS_MACOS
            && ui->bAdd->isHidden() && ui->bDelete->isHidden()
        #endif
            )
    {
        ui->wControlButtons->hide();
    }
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
