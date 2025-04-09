#include "CrashReportDialog.h"

#include "ui_CrashReportDialog.h"

#include <QStyle>

CrashReportDialog::CrashReportDialog(QWidget* parent):
    QDialog(parent),
    ui(new Ui::CrashReportDialog)
{
    ui->setupUi(this);
    ui->bSubmit->setDefault(true);

    auto setErrorState = [this](bool state)
    {
        static const auto infoText = tr(
            "Logs may contain personal information, such as file or folder names. These logs are "
            "not shared with MEGA unless you choose to send them to the Helpdesk for debugging.");
        static const auto errorText = tr(
            "Please describe the issue or attach diagnostic log files to send the error report.");
        ui->wNotificationBanner->setProperty("error", state);
        ui->lInfoBanner->setProperty("error", state);
        ui->lInfoBanner->setText(state ? errorText : infoText);
        ui->teDescribeBug->setProperty("error", state);
        ui->cbAttachLogs->setProperty("error", state);
        this->setStyleSheet(this->styleSheet());
        this->update();
    };
    setErrorState(false);
    connect(ui->bSubmit,
            &QPushButton::clicked,
            this,
            [=]()
            {
                if (ui->teDescribeBug->toPlainText().isEmpty() && !ui->cbAttachLogs->isChecked())
                {
                    setErrorState(true);
                    return;
                }
                else
                {
                    accept();
                }
            });

    connect(ui->teDescribeBug,
            &QTextEdit::textChanged,
            this,
            [=]()
            {
                setErrorState(false);
            });
    connect(ui->cbAttachLogs,
            &QCheckBox::clicked,
            this,
            [=]()
            {
                if (ui->cbAttachLogs->isChecked())
                {
                    setErrorState(false);
                }
            });
}

QString CrashReportDialog::getUserMessage()
{
    return ui->teDescribeBug->toPlainText();
}

bool CrashReportDialog::sendLogs()
{
    return ui->cbAttachLogs->isChecked();
}

CrashReportDialog::~CrashReportDialog()
{
    delete ui;
}
