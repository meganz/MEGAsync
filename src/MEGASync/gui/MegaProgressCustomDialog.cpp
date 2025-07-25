#include "MegaProgressCustomDialog.h"

#include "RefreshAppChangeEvent.h"
#include "ui_MegaProgressCustomDialog.h"

#include <QEvent>

MegaProgressCustomDialog::MegaProgressCustomDialog(QWidget *parent, int minimum, int maximum):
    QDialog(parent),
    ui(new Ui::MegaProgressCustomDialog)
{
    ui->setupUi(this);

    ui->progressBar->setMinimum(minimum);
    ui->progressBar->setMaximum(maximum);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint);
}

MegaProgressCustomDialog::~MegaProgressCustomDialog() {
    delete ui;
}

bool MegaProgressCustomDialog::event(QEvent* event)
{
    if (RefreshAppChangeEvent::isRefreshEvent(event))
    {
        ui->retranslateUi(this);
    }
    return QDialog::event(event);
}
