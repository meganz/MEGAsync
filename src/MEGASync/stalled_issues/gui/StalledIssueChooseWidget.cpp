#include "StalledIssueChooseWidget.h"

#include "MegaApplication.h"
#include "PlatformStrings.h"
#include "StalledIssueHeader.h"
#include "StalledIssuesModel.h"
#include "ui_StalledIssueChooseWidget.h"
#include "Utilities.h"

#include <QMouseEvent>

const int StalledIssueChooseWidget::BUTTON_ID = 0;

StalledIssueChooseWidget::StalledIssueChooseWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::StalledIssueChooseWidget),
    mPathDisableEffect(nullptr)
{
    ui->setupUi(this);

    ui->name->removeBackgroundColor();

    connect(MegaSyncApp->getStalledIssuesModel(), &StalledIssuesModel::showRawInfoChanged, this, &StalledIssueChooseWidget::onRawInfoToggled);
    connect(ui->chooseTitle, &StalledIssueActionTitle::actionClicked, this, &StalledIssueChooseWidget::onActionClicked, Qt::UniqueConnection);

    ui->path->setIndent(StalledIssueHeader::GROUPBOX_CONTENTS_INDENT);
    ui->path->hideLocalOrRemoteTitle();
}

StalledIssueChooseWidget::~StalledIssueChooseWidget()
{
    delete ui;
}

void StalledIssueChooseWidget::setActionButtonVisibility(bool state)
{
    ui->chooseTitle->setActionButtonVisibility(BUTTON_ID, state);
}

void StalledIssueChooseWidget::setMessage(const QString& string, const QPixmap& pixmap, const QString& tooltip)
{
    ui->chooseTitle->setMessage(string, pixmap, tooltip);
}

void StalledIssueChooseWidget::setFailed(bool state, const QString& tooltip)
{
    ui->chooseTitle->setFailed(state, tooltip);
}

void StalledIssueChooseWidget::addDefaultButton()
{
    ui->chooseTitle->addActionButton(QIcon(), tr("Choose"), BUTTON_ID, true);
}

void StalledIssueChooseWidget::onActionClicked(int button_id)
{
    QApplication::postEvent(this, new QMouseEvent(QEvent::MouseButtonPress, QPointF(), Qt::LeftButton, Qt::NoButton, Qt::KeyboardModifier::AltModifier));
    qApp->processEvents();

    emit chooseButtonClicked(button_id);
}

void StalledIssueChooseWidget::setSolved(bool isSolved, bool isSelected)
{
    if (isSolved)
    {
        ui->chooseTitle->setDisable(!isSelected);
        ui->name->setDisable(!isSelected);

        if (!isSelected && !ui->pathContainer->graphicsEffect())
        {
            mPathDisableEffect = new QGraphicsOpacityEffect(this);
            mPathDisableEffect->setOpacity(0.3);
            ui->pathContainer->setGraphicsEffect(mPathDisableEffect);
        }
    }
    else
    {
        ui->chooseTitle->setDisable(false);
        ui->name->setDisable(false);

        ui->pathContainer->setGraphicsEffect(nullptr);
    }
}

QString StalledIssueChooseWidget::chosenString() const
{
    return tr("Chosen");
}

//Generic options
GenericChooseWidget::GenericChooseWidget(QWidget* parent)
    : StalledIssueChooseWidget(parent)
{
    ui->pathContainer->hide();
    ui->nameContainer->hide();

    auto margins(ui->titleContainer->layout()->contentsMargins());
    margins.setTop(4);
    margins.setBottom(4);
    ui->titleContainer->layout()->setContentsMargins(margins);
    ui->chooseTitle->removeBackgroundColor();
}

QString GenericChooseWidget::solvedString() const
{
    return mInfo.solvedText;
}

void GenericChooseWidget::setSolved(bool isSolved, bool isSelected)
{
    if(isSelected)
    {
        QIcon solvedIcon(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
        ui->chooseTitle->setMessage(mInfo.solvedText, solvedIcon.pixmap(16,16));
    }
    else
    {
        ui->chooseTitle->setMessage(QString());
    }

    StalledIssueChooseWidget::setSolved(isSolved, isSelected);
    setActionButtonVisibility(!isSolved);
}

void GenericChooseWidget::setInfo(const GenericInfo &info)
{
    mInfo = info;

    QIcon icon(info.icon);
    auto iconPixmap(icon.pixmap(QSize(16,16)));
    ui->chooseTitle->setHTML(info.title, iconPixmap);
    ui->chooseTitle->addActionButton(QIcon(), info.buttonText, BUTTON_ID, true);
}
