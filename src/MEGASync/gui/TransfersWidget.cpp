#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include <QTextEdit>
#include <QTimer>

TransfersWidget::TransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersWidget)
{
    ui->setupUi(this);

    model = new QTransfersModel();
    tDelegate = new MegaTransferDelegate(this);
    ui->tvTransfers->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tvTransfers->setModel(model);
    ui->tvTransfers->header()->close();

}

TransfersWidget::~TransfersWidget()
{
    delete ui;
}
