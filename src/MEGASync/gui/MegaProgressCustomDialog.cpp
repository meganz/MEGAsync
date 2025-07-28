#include "MegaProgressCustomDialog.h"

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
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    return QDialog::event(event);
}
