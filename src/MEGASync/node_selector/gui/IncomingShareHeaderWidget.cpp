#include "IncomingShareHeaderWidget.h"

#include "megaapi.h"
#include "ui_IncomingShareHeaderWidget.h"

#include <QStyle>
#include <QToolButton>

namespace
{
constexpr auto ACCESS_PROPERTY = "access";
}

IncomingShareHeaderWidget::IncomingShareHeaderWidget(QWidget* parent):
    QWidget(parent),
    ui(new Ui::IncomingShareHeaderWidget)
{
    // A bare QWidget subclass ignores stylesheet background-color/border-radius from its own
    // type selector unless styled-background painting is enabled
    setAttribute(Qt::WA_StyledBackground, true);

    ui->setupUi(this);

    // The chevron opens the context menu for the current share folder, mirroring the chevron on
    // the navigation breadcrumb's last segment. The owner routes it to the current tree view.
    connect(ui->toolButton,
            &QToolButton::clicked,
            this,
            [this]()
            {
                emit menuRequested(
                    ui->toolButton->mapToGlobal(QPoint(0, ui->toolButton->height())));
            });

    clear();
}

IncomingShareHeaderWidget::~IncomingShareHeaderWidget()
{
    delete ui;
}

void IncomingShareHeaderWidget::setData(const IncomingShareHeaderData& data)
{
    ui->folderName->setText(data.folderName);
    ui->userIcon->setIcon(data.userIcon);
    ui->userName->setText(data.userName);
    ui->userEmail->setText(data.userEmail);
    ui->accessIcon->setIcon(data.accessIcon);
    ui->accessLabel->setText(data.accessLabel);

    const auto hasAccessInfo = !data.accessLabel.isEmpty();
    const auto hasOwnerInfo = !data.userName.isEmpty() || !data.userEmail.isEmpty();
    const auto showSeparator = !data.userName.isEmpty() && !data.userEmail.isEmpty();

    ui->accessContainer->setVisible(hasAccessInfo);
    ui->shareeInfo->setVisible(hasOwnerInfo);
    ui->userIcon->setVisible(hasOwnerInfo);
    ui->separator->setVisible(showSeparator);

    ui->accessContainer->setProperty(ACCESS_PROPERTY, data.accessType);
    if (data.accessType == mega::MegaShare::ACCESS_FULL)
    {
        ui->accessIcon->setProperty("normal_off", QLatin1String("support-success"));
    }
    else if (data.accessType == mega::MegaShare::ACCESS_READ)
    {
        ui->accessIcon->setProperty("normal_off", QLatin1String("text-secondary"));
    }
    else if (data.accessType == mega::MegaShare::ACCESS_READWRITE)
    {
        ui->accessIcon->setProperty("normal_off", QLatin1String("text-info"));
    }
    else
    {
        ui->accessIcon->setProperty("normal_off", QLatin1String("text-secondary"));
    }

    setStyleSheet(styleSheet());
}

void IncomingShareHeaderWidget::clear()
{
    setData(IncomingShareHeaderData{});
}
