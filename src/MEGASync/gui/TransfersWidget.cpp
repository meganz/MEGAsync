#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include <QTextEdit>
#include <QTimer>

TransfersWidget::TransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersWidget)
{
    ui->setupUi(this);

    tDelegate = new MegaTransferDelegate(this);
    ui->tvTransfers->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tvTransfers->header()->close();

}

void TransfersWidget::setupTransfers(mega::MegaTransferList *tList, int type)
{
    model = new QTransfersModel(tList, type);
    ui->tvTransfers->setModel(model);
}

TransfersWidget::~TransfersWidget()
{
    delete ui;
}
