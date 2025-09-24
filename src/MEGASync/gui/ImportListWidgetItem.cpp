#include "ImportListWidgetItem.h"

#include "ui_ImportListWidgetItem.h"
#include "Utilities.h"

#include <QFileInfo>

ImportListWidgetItem::ImportListWidgetItem(QString link, int id, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportListWidgetItem)
{
    ui->setupUi(this);
    this->id = id;
    this->link = link;
    this->fileSize = 0;
    this->isFolder = false;
    status = LOADING;
    fileName = tr("Please wait...");
    ui->cSelected->setChecked(false);
    ui->cSelected->setEnabled(false);
    updateGui();
}

ImportListWidgetItem::~ImportListWidgetItem()
{
    delete ui;
}

void ImportListWidgetItem::setData(QString fileName, linkstatus status, long long size, bool isFolder)
{
    this->fileName = fileName;
    this->status = status;
    this->fileSize = size;
    this->isFolder = isFolder;
}

void ImportListWidgetItem::updateGui()
{
    QString name;
    if (fileSize)
    {
        name = fileName + QString::fromLatin1(" (") + Utilities::getSizeString(fileSize) + QString::fromLatin1(")");
    }
    else
    {
        name = fileName;
    }

    ui->lName->ensurePolished();
    ui->lName->setText(ui->lName->fontMetrics().elidedText(name, Qt::ElideMiddle,ui->lName->width()));

    QIcon typeIcon = Utilities::getExtensionPixmap(
        isFolder ? fileName.append(QString::fromUtf8(".folder")) : fileName,
        Utilities::AttributeType::SMALL);

    ui->lImage->setIcon(typeIcon);
    ui->lImage->setIconSize(QSize(24, 24));

    QIcon statusIcon;
    switch(status)
    {
    case LOADING:
        break;
    case CORRECT:
        statusIcon.addFile(QString::fromUtf8(":/images/import_check.svg"),
                           QSize(),
                           QIcon::Normal,
                           QIcon::Off);
        ui->cSelected->setChecked(true);
        ui->cSelected->setEnabled(true);
        break;
    case WARNING:
        statusIcon.addFile(QString::fromUtf8(":/images/alert_triangle.svg"),
                           QSize(),
                           QIcon::Normal,
                           QIcon::Off);
        ui->cSelected->setChecked(false);
        ui->cSelected->setEnabled(false);
        break;
    default:
        statusIcon.addFile(QString::fromUtf8(":/images/import_failed.svg"),
                           QSize(),
                           QIcon::Normal,
                           QIcon::Off);
        ui->cSelected->setChecked(false);
        ui->cSelected->setEnabled(false);
        break;
    }

    ui->lState->setIcon(statusIcon);
    ui->lState->setIconSize(QSize(16, 16));
}

bool ImportListWidgetItem::isSelected()
{
    return ui->cSelected->isChecked();
}

QString ImportListWidgetItem::getLink()
{
    return link;
}

void ImportListWidgetItem::on_cSelected_stateChanged(int state)
{
    emit stateChanged(id, state);
}

void ImportListWidgetItem::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    if (parentWidget())
    {
        setFixedWidth(parentWidget()->width());
    }
}
