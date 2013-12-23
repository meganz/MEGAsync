#include "ImportListWidgetItem.h"
#include "ui_ImportListWidgetItem.h"
#include "utils/Utils.h"

#include <QFileInfo>

ImportListWidgetItem::ImportListWidgetItem(QString link, int id, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ImportListWidgetItem)
{
	ui->setupUi(this);
	this->id = id;
	node = NULL;
	this->link = link;
	status = LOADING;
	fileName = link;
	updateGui();
}

ImportListWidgetItem::~ImportListWidgetItem()
{
	delete ui;
}

void ImportListWidgetItem::setNode(PublicNode *node)
{
	this->node = node;
}

void ImportListWidgetItem::setData(QString fileName, linkstatus status, long long size)
{
	this->fileName = fileName;
	this->status = status;
	this->fileSize = size;
}

void ImportListWidgetItem::updateGui()
{
    if(fileSize) ui->lName->setText(fileName + QString::fromAscii(" (") + Utils::getSizeString(fileSize) + QString::fromAscii(")"));
	else ui->lName->setText(fileName);
    ui->lImage->setPixmap(Utils::getExtensionPixmapSmall(fileName));

	switch(status)
	{
	case LOADING:
		//ui->lState->setText("LOADING");
		break;
	case CORRECT:
        ui->lState->setPixmap(QPixmap(QString::fromAscii("://images/import_ok_icon.png")));
		break;
	case WARNING:
        ui->lState->setPixmap(QPixmap(QString::fromAscii("://images/import_warning_ico.png")));
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	default:
        ui->lState->setPixmap(QPixmap(QString::fromAscii("://images/import_error_ico.png")));
		ui->cSelected->setChecked(false);
		ui->cSelected->setEnabled(false);
		break;
	}
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
