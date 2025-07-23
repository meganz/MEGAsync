#include "RemoteItemUi.h"

#include "RefreshAppChangeEvent.h"
#include "ui_RemoteItemUi.h"

RemoteItemUi::RemoteItemUi(QWidget* parent):
    QWidget(parent),
    ui(new Ui::RemoteItemUi)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    setUsePermissions(false);
#else
    connect(ui->bPermissions, &QPushButton::clicked, this, &RemoteItemUi::permissionsClicked);
#endif

    connect(ui->bAdd,
            &QPushButton::clicked,
            this,
            [this]()
            {
                emit addClicked();
            });

    ui->bAdd->setAutoDefault(true);
}

RemoteItemUi::~RemoteItemUi()
{
    delete ui;
}

void RemoteItemUi::setTitle(const QString& title)
{
    ui->groupBox->setTitle(title);
}

void RemoteItemUi::initView(QTableView* newView)
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

    if (ui->bPermissions->isHidden() && ui->bAdd->isHidden())
    {
        ui->wControlButtons->hide();
    }
}

QTableView* RemoteItemUi::getView()
{
    return ui->tableView;
}

bool RemoteItemUi::event(QEvent* event)
{
    if (RefreshAppChangeEvent::isRefreshEvent(event))
    {
        ui->retranslateUi(this);
    }

    return QWidget::event(event);
}

void RemoteItemUi::setTableViewProperties(QTableView* view) const
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

void RemoteItemUi::setAddButtonEnabled(bool enabled)
{
    ui->bAdd->setEnabled(enabled);
}
