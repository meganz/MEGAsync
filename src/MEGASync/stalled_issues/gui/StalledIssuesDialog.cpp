#include "StalledIssuesDialog.h"
#include "ui_StalledIssuesDialog.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssuesProxyModel.h"
#include "StalledIssueDelegate.h"

#include "Utilities.h"

const char* ITS_ON = "itsOn";

StalledIssuesDialog::StalledIssuesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StalledIssuesDialog),
    mCurrentTab(ALL_ISSUES_TAB),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr))
{
    ui->setupUi(this);

    auto model = new StalledIssuesProxyModel(this);
    model->setSourceModel(MegaSyncApp->getStalledIssuesModel());

    ui->stalledIssuesTree->setModel(model);
    mViewHoverManager.setView(ui->stalledIssuesTree);

    auto delegate = new StalledIssueDelegate(model, ui->stalledIssuesTree);
    ui->stalledIssuesTree->setItemDelegate(delegate);

    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::stalledIssuesCountChanged,
            this,  &StalledIssuesDialog::onStalledIssuesModelCountChanged);

    MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();

    //Init all categories
    mTabFramesToggleGroup[ALL_ISSUES_TAB] = ui->allIssuesTag;
    QIcon tagIcon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/sidebar_failed_ico.png"));
    ui->allIssuesIcon->setPixmap(tagIcon.pixmap(ui->allIssuesIcon->size()));

    mTabFramesToggleGroup[NAME_CONFLICTS_TAB] = ui->nameTag;
    tagIcon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/sidebar_failed_ico.png"));
    ui->nameIcon->setPixmap(tagIcon.pixmap(ui->nameIcon->size()));

    mTabFramesToggleGroup[ITEM_TYPE_TAB] = ui->itemTypeTag;
    tagIcon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/sidebar_failed_ico.png"));
    ui->itemTypeIcon->setPixmap(tagIcon.pixmap(ui->itemTypeIcon->size()));

    mTabFramesToggleGroup[OTHER_TAB] = ui->otherTag;
    tagIcon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/sidebar_failed_ico.png"));
    ui->otherIcon->setPixmap(tagIcon.pixmap(ui->otherIcon->size()));

    for (auto tabFrame : qAsConst(mTabFramesToggleGroup))
    {
        tabFrame->setProperty(ITS_ON, false);
        tabFrame->installEventFilter(this);
    }

    toggleTab(ui->allIssuesTag);

    QColor shadowColor ("#E5E5E5");
    mShadowTab->setParent(this);
    mShadowTab->setBlurRadius(5);
    mShadowTab->setXOffset(0);
    mShadowTab->setYOffset(0);
    mShadowTab->setColor(shadowColor);
    mShadowTab->setEnabled(true);
}

StalledIssuesDialog::~StalledIssuesDialog()
{
    delete ui;
}

bool StalledIssuesDialog::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
       toggleTab(obj);
    }

    return QDialog::eventFilter(obj, event);
}

void StalledIssuesDialog::on_doneButton_clicked()
{
    close();
}

void StalledIssuesDialog::on_updateButton_clicked()
{
    MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
}

void StalledIssuesDialog::onStalledIssuesModelCountChanged()
{
    auto isEmpty = MegaSyncApp->getStalledIssuesModel()->rowCount(QModelIndex()) == 0;
    ui->TreeViewContainer->setCurrentWidget(isEmpty ? ui->EmptyViewContainerPage : ui->TreeViewContainerPage);
}

void StalledIssuesDialog::toggleTab(QObject* toggledObj)
{
    if(auto toggledWidget = dynamic_cast<QWidget*>(toggledObj))
    {
        auto tab = mTabFramesToggleGroup.key(toggledWidget,ALL_ISSUES_TAB);

        mTabFramesToggleGroup[mCurrentTab]->setProperty(ITS_ON, false);
        mCurrentTab = tab;
        toggledWidget->setProperty(ITS_ON, true);
        toggledWidget->setGraphicsEffect(mShadowTab);

        ui->categories->setStyleSheet(ui->categories->styleSheet());

        //Change filter
    }
}
